<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/5/14
 * Time: 7:38 PM
 */

/*
 * angelreg:  controller class for registration.  this file also contains the corresponding view.
 *
 * Class which displays registration screen and sends a verification email.
 */
class angelreg {

    public $view;

    function __construct() {
        $this->view = new angelreg_view();
    }

    /*******************
     * This method is the standard reg page
     */
    function showRegPage($p, $error) {

        $body = $this->view->regPageBody($p, $error);
        $this->view->outputHTML($body);
    }
    /*******************
     * This method is the password only reg page
     */
    function showPassonlyPage($p) {

        $body = $this->view->passonlyPageBody($p);
        $this->view->outputHTML($body);
    }

    /**********************
     * This method takes as input a new person to store in the DB. It is a user error if the entry (email is the key)
     * is already taken by a registration (a password is set), otherwise we let the registrant take
     * over the entry - once they verify the email, of course.
     */
    function saveReg($p) {

        $conn = $p->connect();
        if (!$conn) {
            angeldie($p->error());
        }
        $log = new angellogin();
        /* just a note here that I should put the POST password into $p and not delve into the web cgi vars here */
        $p->password = $log->password_hash($_POST['password']);     /* store the hashed password */
        if ($p->password == NULL)
            angeldie("Password hash failed!");

        /*
         * See if an entry exists with this email -- if it does we take it over if there is not already a
         * registration there (password set), otherwise we write a new entry
         */
        $existingPerson = $p->findMatchingPerson(true);
        if ($existingPerson != NULL) {
            if ($existingPerson->password != "" ) {
                angellog("registration failed, dup email: " . $existingPerson->email . ".");
                $this->showRegPage($p, "An account with this email address already exists.");
                return false;
            }

            $existingPerson->hash = md5( rand(0,1000) );             // Generate random 32 character hash
            $existingPerson->state = 'waitverify';                   // waiting for email verification
            $existingPerson->regdate = date( 'Y-m-d H:i:s');        // save reg date
            $existingPerson->kind = 'user';

            /*
             * there is now a password only reg form, so only set email and cell if they are in $p.
             */

            if ($p->email)
                $existingPerson->email = $p->email;
            $existingPerson->password = $p->password;
            if ($p->cell)
                $existingPerson->cell = $p->cell;
            $existingPerson->regdate = date( 'Y-m-d H:i:s');   // save reg date

            $vars = get_object_vars($existingPerson);                // place values in assoc. array so we can create sql query
            // angeldie(serialize($vars));                           // debug

            /* don't unset id, since that is how mysql update knows which row to update */
            unset($vars["last_update"]);
            $sql = ik12_arrayToSql($conn, "persontable", $vars, "UPDATE");
            $r = mysqli_query($conn, $sql);
            if (!$r)
                angeldie("reg UPDATE Failed: " . $sql . mysqli_error($conn));

            angellog("successful overwrite registration for: " . $existingPerson->email . ".");
            /*
             * Set the id and hash in the person which will be used to send a verification email
             */
            $p->hash = $existingPerson->hash;
            $p->id = $existingPerson->id;

        } else {
            /*
             * This is a brand new registration. Insert into the persontable and ask to verify their email.  We will
             * set the default kind to 'founder'
             */
            $p->hash = md5( rand(0,10000) );             // Generate random 32 character hash
            $p->state = 'waitverify';                   // waiting for email verification
            $p->regdate = date( 'Y-m-d H:i:s');   // save reg date
            $p->kind = 'user';


            $vars = get_object_vars($p);                // place values in assoc. array so we can create sql query
            //angeldie(serialize($vars));                // debug
            unset($vars["id"]);
            unset($vars["last_update"]);
            $sql = ik12_arrayToSql($conn, "persontable", $vars, "INSERT INTO");
            // angeldie($sql);                           // debug
            $r = mysqli_query($conn, $sql);
            if (!$r)
                angeldie("INSERTION Failed into peopletable: " . mysqli_error($conn));

            angellog("successful new registration for: " . $p->email . ".");

            $existingPerson = $p->findMatchingPerson(true); /* look up the person to get the correct id */
            $p->id = $existingPerson->id;
        }
        return true;
    }



    /***********************
     * this method sends a verify email and confirms that it was sent. It assumes the input person
     * is already in the database with their hash set.
     */
    function sendVerifyEmail($p) {
        // angeldie(serialize($p));    // debug
        $path = $GLOBALS["path"];

        $subject    = 'Angel Modeling Email Verification';
        $body ="
Thanks for signing up!

Your account has been created and you can login with $p->email and your password after you have activated your account by clicking on the url below.

http://angelcalc.com/reg?verify=1&id=" . $p->id . '&hash=' . $p->hash . "

Sincerely,
Geoff";

        angelSendEmail($subject, $body, $p->firstname,$p->email);

        $body = $this->view->verifyEmailSentBody();
        $this->view->outputHTML($body);

    }



    function verifyReg($p) {

        $conn = $p->connect();
        if (!$conn) {
            angeldie("verify: " . $p->error());
        }

        /*
         * Does an entry exist with this id?
         */
        $existingPerson = $p->findMatchingPerson(true);
        if ($existingPerson == NULL)
            angeldie("Could not find the person with id: " . $p->id);

        if ($existingPerson->hash != $p->hash) {
            angelerror("Your verification failed. Please contact Angelcalc (support@angelcalc.com).");
        }
        /*
         * Okay, the email is verified. Write the db with a new status and send them to login.
         */
        $q = sprintf("UPDATE persontable SET state='verified' WHERE id=%s", $p->id);
        // angeldie($q);  // debug
        $r = mysqli_query($conn, $q);
        if (!$r)
            angeldie("reg verify UPDATE Failed: " . mysqli_error($conn));

        angellog("successful email (" . $existingPerson->email . ") verification.");
        header("Location: login?verified=1");

    }

}    // class angelreg

/*
 * View for angelreg
 */

class angelreg_view extends angelview {


    function __construct() {
        parent::__construct();

        $path = $GLOBALS["path"];

        $this->htmlHeader = "<title>AngelCalc Registration</title>";
        $this->header = "logoOnlyHeader.inc";
        $this->js .= "<script src='$path/jscript/livevalidation_standalone.js'></script>";
        $this->footer       = "angelfooter.inc";



    }

    function regPageBody($p, $error) {

        $body = "<div id='wrap'>
<p style='margin-top:10px;margin-bottom:15px;color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;text-align:center'>Registration</p>
                                <p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p>";

        if ($error != "")
            $body .= "<p class='ik12rlError'>$error</p>";
        else
            $body .= "<p style='font-size:115%'>Please begin by registering for an account</p>";

        $getVars = http_build_query($_GET);

        $path = $GLOBALS["path"];
        $body .= "

            <form action='$path/reg' method='post'>
                    <input type='hidden' name='saveVars' value='$getVars'>
                    <table>
                        <tr><td><label for='firstname'>First Name:</label></td>
                            <td><div id='regTable'>

                                    <input type='text' name='firstname' id='f22' value='$p->firstname' tabindex='1' />

                                    <script type='text/javascript'>
                                        var f22 = new LiveValidation('f22', {validMessage: ' '});
                                        f22.add( Validate.Presence, {failureMessage: 'First name is required.'} );
                                    </script>
                             </div></td>
            <td><label for='password'>Password:</label>
                            <td> <div id='regTable'>
                                    <input type='password' name='password' id='myPasswordField' value='' tabindex='5' />
                                </div></td>
            </tr>
            <tr><td><label for='lastname'>Last Name:</label></td>
                <td><div id='regTable'>
                   <input type='text' name='lastname' id='f21' value='$p->lastname' tabindex='2' />


                    <script type='text/javascript'>
                        var f21 = new LiveValidation('f21', {validMessage: ' '});
                        f21.add( Validate.Presence, {failureMessage: 'Last name is required.'} );
                    </script>
                 </td>
                <td><label for='cell'>Confirm Password</label></td>
                            <td><div id='regTable'>
                                    <input type='password' name='passwordconf' id='f19' value='' tabindex='6' />
                                    <script type='text/javascript'>
                                        var f19 = new LiveValidation('f19', {validMessage: ' '});
                                        f19.add( Validate.Presence, {failureMessage: 'Please enter a password.'} );
                                        f19.add( Validate.Length,  { minimum: 4, maximum: 255, tooShortMessage:'Passwords must be at least 4 characters!' } );
                                        f19.add(Validate.Confirmation, { match: 'myPasswordField', failureMessage: 'Passwords do not match!' } );

                                    </script>
                                </div></td>
            </tr>

            <tr><td><label for='email'>Email:</label></td>
                <td><div id='regTable'>

                        <input type='text' name='email' id='f20' value='$p->email' tabindex='3' />

                        <script type='text/javascript'>
                            var f20 = new LiveValidation('f20', {validMessage: ' '});
                            f20.add( Validate.Presence, {failureMessage: 'Please enter your email.'} );
                            f20.add(Validate.Email, {failureMessage: 'Invalid Email!'} );
                        </script>
                    </div></td>



                        <tr><td colspan='4'>
                                <input type='submit' class='submit_button' value='Sign up' tabindex='8'></td></tr>

                        </table>
                        </form>
                        <!-- end registration.  Let them login or deal with forgotton password below -->
                        <div style='text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;'>
                            <a href='login'>Already registered? Login Here</a> |
                            <a href='passrecover'>Forgot Password?</a>
                        </div>


                    </div>
                    <!-- end wrap div -->
                    ";



        return $body;
    }

    function passonlyPageBody($p) {

        $body = "<div id='wrap'>
<p style='margin-top:10px;margin-bottom:15px;color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;text-align:center'>Registration</p><p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p>
              <p style='font-size:115%'>Please choose a password for your account</p>";

        $path = $GLOBALS["path"];
        $body .= "

            <form action='$path/reg' method='post'>
                    <table>


                        <tr> <td><label for='password'>Password:</label>
                            <td> <div id='regTable'>
                                    <input type='password' name='password' id='myPasswordField' value='' tabindex='5' />
                                </div></td>
                            <td><label for='cell'>Confirm Password</label></td>
                            <td><div id='regTable'>
                                    <input type='password' name='passwordconf' id='f19' value='' tabindex='6' />
                                    <script type='text/javascript'>
                                        var f19 = new LiveValidation('f19', {validMessage: ' '});
                                        f19.add( Validate.Presence, {failureMessage: 'Please enter a password.'} );
                                        f19.add( Validate.Length,  { minimum: 4, maximum: 255, tooShortMessage:'Passwords must be at least 4 characters!' } );
                                        f19.add(Validate.Confirmation, { match: 'myPasswordField', failureMessage: 'Passwords do not match!' } );

                                    </script>
                                </div></td></tr>

                        <tr><td colspan='4'>
                                <input type='submit' class='submit_button' value='Save Password' tabindex='8'></td></tr>

                        </table>
                        <input type='hidden' name='passonly' value='1'>
                        <input type='hidden' name='id' value='$p->id'>
                        </form>
                        <!-- end registration.  Let them login or deal with forgotton password below -->
                        <div style='text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;'>
                            <a href='/login'>Already registered? Login Here</a> |
                            <a href='/passrecover'>Forgot Password?</a>
                        </div>


                    </div>
                    <!-- end wrap div -->
                    ";



        return $body;
    }

    function verifyEmailSentBody() {
        $body = "<div id='wrap'>
<p style='margin-top:10px;margin-bottom:15px;color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;text-align:center'>Registration</p><p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p>
                  <p style='font-size:115%;text-align:left;'>Your registration was successful. In order to log in and begin your application, please check your inbox for a verification email.</p>
                         <div style='text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;'>
                            <a href='login'>Login now</a>
                        </div>

                        </div>";

        return $body;
    }

}

