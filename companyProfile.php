<!DOCTYPE html>
<html><head><title>IK12 Company Profile</title>
    <meta http-equiv="Content-Type" content="text/html;charset=UTF-8"></head>

<link type="text/css" rel="stylesheet" href="/css/ik12.css"/>
<link type="text/css" rel="stylesheet" href="/css/bookface.css"/>
<script src="/jscript/jquery-1.11.1.min.js">
</script>


<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 8/26/14
 * Time: 10:37 AM
 */

require "search_autocomplete.inc";  // datalist id must be "txtHint"
require "ik12objects.inc";
require "ik12_misc.inc";                // include misc useful functions. put more stuff here!

$auth = ik12Authenticate("ik12founder"); /* check to make sure we are logged in */

if ($auth == "root")
    require "header_w_search.inc";
else
    require "header_basic.inc";



if (array_key_exists('id', $_GET)) {
    $id = $_GET['id'];
} elseif (array_key_exists('id', $_POST)) {
    $id = $_POST['id'];
} else
    ik12die("no company input");

if ($_SESSION["loggedInUserCompanyID"] != $id and $_SESSION["auth"] != "root")
    ik12error("You may not edit someone else's company profile!");


/*
 * Connect to the DB and find the company with that id.
 */

$co = new ik12company();
$conn = $co->connect();
if (!$conn)
    ik12die('Could not connect: ' . $co->error());

$co->id = $id;

$foundCompany = $co->findMatchingCompany();
if ($foundCompany == NULL)
    ik12die("No company found with id: $id");

if ($foundCompany->logo == NULL or $foundCompany->logo == "")
    $logo = "/img/imaginek12.png";
else
    $logo     = $foundCompany->logo;

$error = "";
$message = "";
if (array_key_exists('commit', $_POST)) {
    /*
     * "Update" has been clicked.  Get data and update, then display form again
     */
    // ik12die(serialize($_POST));  // debug
    $name = mysqli_escape_string($conn,$_POST['company']['name']);
    $shortdesc = mysqli_escape_string($conn,$_POST['company']['shortdesc']);
    $fulldesc = mysqli_escape_string($conn,$_POST['company']['fulldesc']);
    $email = mysqli_escape_string($conn, $_POST['company']['email']);
    $url = mysqli_escape_string($conn,$_POST['company']['url']);
    $location = mysqli_escape_string($conn,$_POST['company']['location']);
    $cohort = $_POST['company']['cohort'];
    $fileName = $_FILES['logo']['name'];

    if ($cohort == "") {
        $cohort = "xxxxx";  // storage location all non-ImagineK12 companies
    }

    if ($fileName != "") {
        /*
         * Really need to do a lot of error checking here. For now, just move the file and assume all is ok.
         * Also, it is a big ugly to assume we are one directory below the img directory, but this seems
         * easiest for now...probably should do this off wherever the "document" root is ('/img' does not work).
         */

        $n = str_replace(' ', '', $name);  // strip out whitespace

        if (preg_match('/jpg|jpeg/',$fileName)){
            $imageType = "jpg";
        } elseif (preg_match('/png/',$fileName)){
            $imageType = "png";
        } else
            $error = "Illegal image format: only jpegs and pngs are supported.";


        if ($error == "") {
            $filename = sprintf("../img/logos/%s/%s_%s.%s", $cohort, $n, $id,$imageType);
            $thumbFilename = sprintf("../img/logos/%s/%s_%s-thumb.%s", $cohort, $n, $id, $imageType);
            if (!move_uploaded_file($_FILES['logo']['tmp_name'], $filename)) {
                ik12die("move file failed: " . print_r($_FILES, true));
            }
            // $logo = sprintf("/img/logos/%s/%s_%s.jpg", $cohort, urlencode($n), $id);
            $logo = sprintf("/img/logos/%s/%s_%s-thumb.%s", $cohort, urlencode($n), $id,$imageType);
            ik12_makeThumb($filename, $thumbFilename, 400);   // arbitrarily choosing 400 in the hopes that it will look decent
        }
    }

    if ($error == "") {
        $q = sprintf("UPDATE companytable SET name='%s',shortdesc='%s',description='%s',email='%s',url='%s',location='%s', logo='%s' WHERE id=%s",
                                               $name,    $shortdesc,   $fulldesc,       $email,    $url,    $location,     $logo,          $id);
        //ik12die($q);    // debug

        $r = mysqli_query($conn, $q);
        if (!$r)
            ik12die("useredit_company UPDATE Failed: " . mysqli_error($conn) . " query is: " . $q);
        else {
            $message = "Company profile successfully updated.";
            ik12log("Company profile for $name ($id) updated.");
        }
    }
    /*
     * When I move this to a view this will be better, but now drop down and display these fields again, and they need to be html escaped.
     */
    $name = htmlentities($_POST['company']['name'], ENT_QUOTES);
    $shortdesc = htmlentities($_POST['company']['shortdesc'], ENT_QUOTES);
    $fulldesc = htmlentities($_POST['company']['fulldesc'], ENT_QUOTES);
    $email = htmlentities($_POST['company']['email'], ENT_QUOTES);
    $url = htmlentities($_POST['company']['url'], ENT_QUOTES);
    $location = htmlentities($_POST['company']['location'], ENT_QUOTES);


} else {
    /*
     *  Display the form with the company's information
     */
    // ik12die($foundCompany->name);
    $name           = htmlentities($foundCompany->name, ENT_QUOTES);
    $shortdesc      =  htmlentities($foundCompany->shortdesc, ENT_QUOTES);
    $fulldesc       = htmlentities($foundCompany->description, ENT_QUOTES);
    $email          = htmlentities($foundCompany->email, ENT_QUOTES);
    $url            = htmlentities($foundCompany->url, ENT_QUOTES);
    $location       = htmlentities($foundCompany->location, ENT_QUOTES);
    $cohort         = $foundCompany->cohort;
    $id             = $foundCompany->id;
}


echo "
        <div class='container'>
        <div class='content nomargin'>
        <div class='row toprow'>
        <div class='col-md-3'>
        <h2>$name's profile</h2>
        <img alt='logo' src='$logo' width='200' />
        <h4 style='color:red'>$error</h4>
        <h4 style='color:blue'>$message</h4>
        </div>
        <div class='col-md-9'>
        <div class='section-pad'>
        <form accept-charset='UTF-8' action='/dbm/useredit_company.php?id=$id' class='form-horizontal' enctype='multipart/form-data' id='edit_company_823' method='post'>
        <input name='company[cohort]' type='hidden' value='$cohort' />
        <div style='display:none'><input name='utf8' type='hidden' value='&#x2713;' /><input name='_method' type='hidden' value='patch' /><input name='authenticity_token' type='hidden' value='fI3i9x2AqDbzSF+jUclCWbRi/jtiYuyySB74cj/vmgg=' /></div>
        <fieldset>
        <div class='form-group'>
        <label class='control-label col-md-3' for='name'>Name</label>
        <div class='col-md-9'>
        <input class='form-control' id='name' name='company[name]' type='text' value='$name' />
        </div>
        </div>
        <div class='form-group'>
        <label class='control-label col-md-3' for='brief_desc'>Brief Description</label>
        <div class='col-md-9'>
        <input class='form-control' id='company_shortdesc' name='company[shortdesc]' type='text' value='$shortdesc' />
        </div>
        </div>
        <div class='form-group'>
        <label class='control-label col-md-3' for='full_desc'>Full Description</label>
        <div class='col-md-9'>
        <textarea class='form-control' id='company_fuldesc' name='company[fulldesc]'>$fulldesc</textarea>
        </div>
        </div>
        <div class='form-group'>
        <label class='control-label col-md-3' for='email'>Email</label>
        <div class='col-md-9'>
        <input class='form-control' id='company_email' name='company[email]' type='text' value='$email' />
        </div>
        </div>
        <div class='form-group'>
        <label class='control-label col-md-3' for='url'>URL</label>
        <div class='col-md-9'>
        <input class='form-control' id='company_url' name='company[url]' type='text' value='$url' />
        </div>
        </div>
        <div class='form-group'>
        <label class='control-label col-md-3' for='location'>Location</label>
        <div class='col-md-9'>
        <input class='form-control' id='company_cell' name='company[location]' type='text' value='$location' />
        ";

if ($location != "") {
    echo "<iframe width=500 height=300 frameborder='0' style='border:0'
          src='https://www.google.com/maps/embed/v1/place?q=$location&key=AIzaSyDpVvmr-OvUw3Rktt-w3ZsL5VHYjHDvhdk'></iframe>";
}

echo "


        </div>
        </div>
        <div class='form-group'>
        <label class='control-label col-md-3' for='logo'>Your Logo</label>
        <div class='col-md-9'>
        <input id='company_logo' name='logo' type='file' value='$logo' />
        <div class='help-block'>
        <strong>Important!</strong>
        Please use a large (at least 1000px wide) photo with your logo clearly visible.
        </div>
        </div>
        </div>

        <div class='form-group'>
        <div class='col-md-3'></div>
        <div class='col-md-9'>
        <input class='btn btn-primary btn-lg' name='commit' type='submit' value='Update' />";

        if ($cohort == 'xxxxx')         // special case for Imagine K12's imaginary cohort
            echo "<a href='/bookface.php'>Return to Bookface </a>";
        else
            echo "<a href='/bookface.php?cohort=$cohort'>Return to Bookface </a>";

echo "
        </div>
        </div>
        </fieldset>
        </form>

        </div>

        </div>
        </div>

        </div>
    ";

?>