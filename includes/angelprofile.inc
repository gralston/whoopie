<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/8/14
 * Time: 11:04 AM
 */

class angelProfile {

    public $error = "";         // error message to display
    public $message = "";       // (non-error) message to display
    public $view;

    function __construct() {
        $this->view = new angelProfileView();
    }

    /*******************
     * This method is the standard reg page
     */
    function showUProfilePage($investorID, $p, $co) {

        $this->view->error = $this->error;
        $this->view->message = $this->message;

        $body = $this->view->userProfilePageBody($investorID, $p, $co);
        $this->view->outputHTML($body);
    }

    function showCompanyPage($co, $investorID) {

        $this->view->error = $this->error;
        $this->view->message = $this->message;

        $body = $this->view->companyPageBody($co, $investorID);
        $this->view->outputHTML($body);
    }

    function showFundPage($fund, $investorID) {

        $this->view->error = $this->error;
        $this->view->message = $this->message;

        $body = $this->view->fundPageBody($fund, $investorID);
        $this->view->outputHTML($body);
    }

    function showCompanyProfile($co, $investorID, $prospect) {

        $this->view->error = $this->error;
        $this->view->message = $this->message;

        $body = $this->view->companyProfileBody($co, $investorID, $prospect);
        $this->view->outputHTML($body);
    }
    function showFundProfile($fund, $investorID, $prospect) {

        $this->view->error = $this->error;
        $this->view->message = $this->message;

        $body = $this->view->fundProfileBody($fund, $investorID, $prospect);
        $this->view->outputHTML($body);
    }

    function addFounder($co, $p) {
        if ($co->founder_ids == "")
            $co->founder_ids = "$p->id";
        else
            $co->founder_ids .= "," . "$p->id";

        $co->updateThisRow();
    }

    /*************************************************************
     *  create and return a new prospsective company to invest in
     *     actually create a dummy investment with type "prospect" and
     *     only create a company if it wasn't already in our DB. Return
     *     the company!
     * @param $co
     * @param $investorID
     */
    function createNewProspect($co, $investorID) {
        /*
         * The prospect is either a company that exists in our DB or a new one. Handle both cases. A new company
         * will not have an id whereas an existing company will id.
         */
        $id = 0;
        if (array_key_exists('id', $_GET))
            $id = $_GET['id'];
        if ($id == 0) {     // new company
            $co->creatorID = $investorID;
            $this->error = $this->addOrUpdateCompany($co, $investorID, false);      // the false indicates not updating!
            if ($this->error == "") {
                $co->name = $_POST["companyName"];
                $foundCompany = $co->findMatchingCompany();
                $this->message = addslashes("New prospect: $foundCompany->name created and added");
            }
        } else {
            $co->id = $id;
            $foundCompany = $co->findMatchingCompany(true);
            if ($foundCompany == NULL)
                angeldie("companyPage - new prospect: No company found with id: $id");
            $this->error = $this->addOrUpdateCompany($co, $investorID, true);      // update any changes
            if ($this->error == "")
                $this->message = addslashes("New prospect: $foundCompany->name added");
        }
        /*
         * Create the prospect investment and write it to the db. Wonder how we convert prospects to real investments?
         */
        if ($this->error == "") {
            $inv = new angelInvest();
            $inv->company_id = $foundCompany->id;
            $inv->investor_id = $investorID;
            $inv->status = "prospect";
            $inv->invest_amount = $_POST["proposedInvestment"];   // where do they enter this?? !!!
            $inv->insertRow();
        }
        return($foundCompany);
    }

    function addOrUpdateUProfile($p) {
        /*
        * "Update" the user profile from the POST'd data
        */

        // angeldie("here");
        $this->error = "";
        if ($p->email == "")
            $updating = false;      // this is a new person
        else
            $updating = true;

        $id = $p->id;

        $path = $GLOBALS["path"];
        $opass = ""; $npass = ""; $cpass = "";

        $p->firstname = $_POST['user']['firstname'];
        $p->lastname = $_POST['user']['lastname'];
        $p->email = $_POST['user']['email'];
        $p->cell = $_POST['user']['cell'];
        $p->bio = $_POST['user']['bio'];
        if ($p->kind == "user") {
            $opass = $_POST['user']['old_password'];
            $npass = $_POST['user']['new_password'];
            $cpass = $_POST['user']['conf_password'];
        }

        $p->github = $_POST['user']['github'];
        $p->linkedin = $_POST['user']['linkedin'];
        $p->twitter = $_POST['user']['twitter'];
        $p->facebook = $_POST['user']['facebook'];


        // angeldie(serialize($_FILES));        // debug
        $fileName = $_FILES['photo']['name'];


        if ($p->photo == NULL)
            $p->photo = "/img/founder.jpeg";

        if ($p->cell != "")
            $p->cell = angelformatPhoneNumber($p->cell);


        if ($fileName != "") {
            // angeldie("got a photo: " . $fileName);   // debug
            /*
             * Really need to do a lot of error checking here. For now, just move the file and assume all is ok.
             * Also, it is a bit ugly to assume we are one directory below the img directory, but this seems
             * easiest for now...probably should do this off wherever the "document" root is ('/img' does not work).
             */

            $imageType = angelImageType($fileName);

            if ($imageType == "") {
                $this->error = "Illegal image format: only jpegs and pngs are supported.";
            }


            if ($this->error == "") {
                $fn = str_replace(' ', '', $p->firstname);  // strip out whitespace
                $ln = str_replace(' ', '', $p->lastname);   // strip out whitespace
                $filename = sprintf("img/photos/%s_%s_%s.%s", $fn, $ln, $p->id,$imageType);
                $thumbFilename = sprintf("img/photos/%s_%s_%s-thumb.%s", $fn, $ln, $p->id,$imageType);
                if (!move_uploaded_file($_FILES['photo']['tmp_name'], $filename)) {
                    angeldie("move file failed: " . $filename . " from " . print_r($_FILES, true));
                }

                $p->photo = sprintf("$path/img/photos/%s_%s_%s-thumb.%s", urlencode($fn), urlencode($ln), $p->id,$imageType);
                angelMakeThumb($filename, $thumbFilename, 360);   // arbitrarily choosing 360 width in the hopes that it will look decent
            }
        }


        /*
         * handle a password update
         */
        $pass = $p->password;
        if ($opass != "" and $npass != "") {

            if (strlen($npass) < 4)
                $this->error = "Passwords must be at least 4 characters in length.";
            elseif ($npass != $cpass)
                $this->error = "Passwords do not match.";
            else {
                $log = new angellogin();

                if ($log->password_verify($opass, $pass) or     // password is correct ...
                    ($opass == "We<3edtech!" and $_SESSION["auth"] == "root" )) {    //  ... or this is an authorized user with the right magic password
                    $p->password = $log->password_hash($npass);     /* store the hashed password */
                    if ($p->password == NULL)
                        angeldie("Password hash failed!");
                } else
                    $this->error = "Incorrect Password, please try again.";
            }

        }
        /* I think it is a bad idea to reset kind here, so I am punting on it */
        /*
        $kind = $auth;          // just in case a founder's kind is set wrong (it happens!)

        if ($foundPerson->kind == "angelPerson")     // but don't reset our kind!
            $kind = 'angelPerson';
        */

        if ($this->error == "") {
            if (! $updating) {
                $id = $p->insertRow();
            } else {


                $conn = $p->returnconn();
                $q = sprintf("UPDATE persontable SET firstname='%s',lastname='%s',email='%s',cell='%s',photo='%s',password='%s',bio='%s',edu='%s',
                                                                                           work='%s', edu_interest='%s',impressive='%s',
                                                                                           github='%s',linkedin='%s',twitter='%s',facebook='%s',website='%s'
                                            WHERE id=%s",
                    $p->firstname,   $p->lastname,    $p->email,    $p->cell,    $p->photo,    $p->password,
                    mysqli_escape_string($conn, $p->bio),
                    mysqli_escape_string($conn, $p->edu),
                    mysqli_escape_string($conn, $p->work),
                    mysqli_escape_string($conn, $p->edu_interest),
                    mysqli_escape_string($conn, $p->impressive),
                    $p->github,$p->linkedin,$p->twitter,$p->facebook,$p->website,$p->id);

                // angeldie($q);    // debug
                /*
                 * I should move this function to ik12db and remove all mysqli from here
                 */

                $r = mysqli_query($conn, $q);
                if (!$r)
                    angeldie("useredit_person UPDATE Failed: " . mysqli_error($conn));
                else {
                    $this->message = "Profile successfully updated.";
                    angellog("Profile for $p->firstname $p->lastname ($p->id) updated.");
                }

            }
        }

        return($id);
    }

    function impersonateUser($p) {

        $currFname = $_SESSION['loggedInUserFname'];
        $currUID = $_SESSION['loggedInUserID'];
        angellog("$currFname ($currUID) is impersonating $p->firstname $p->lastname ($p->id)");
        /*
         * first save your current state (we should have a better way of doing this!)
         */
        $_SESSION["SAVEDloggedInUserFname"] = $_SESSION["loggedInUserFname"];
        $_SESSION["SAVEDloggedInUserID"] = $_SESSION["loggedInUserID"];
        $_SESSION["SAVEDloggedInUserCompanyID"] = $_SESSION["loggedInUserCompanyID"];
        /*
         * now switch to the impersonated person
         */

        $_SESSION["loggedInUserFname"] = $p->firstname;
        $_SESSION["auth"] = $p->kind;
        $_SESSION["loggedInUserID"] = $p->id;
        $_SESSION["loggedInUserCompanyID"] = $p->company_id;
        $_SESSION["impersonating"] = true;


    }       // impersonateUser

    function stopImpersonatingUser() {

        if (@$_SESSION["impersonating"]) {
            /*
             * Switch back to our saved state and we are no longer impersonating
             */
            $impFname = $_SESSION["loggedInUserFname"];
            $impUID = $_SESSION["loggedInUserID"];
            $_SESSION["loggedInUserFname"] = $_SESSION["SAVEDloggedInUserFname"];
            $_SESSION["auth"] = "root";
            $_SESSION["loggedInUserID"] = $_SESSION["SAVEDloggedInUserID"];
            $_SESSION["loggedInUserCompanyID"] = $_SESSION["SAVEDloggedInUserCompanyID"];
            $_SESSION["impersonating"] = false;
            $savedFname = $_SESSION['SAVEDloggedInUserFname'];
            $savedUID = $_SESSION['SAVEDloggedInUserID'];

            angellog("$savedFname ($savedUID) has stopped impersonating $impFname  ($impUID)");

        }

    }       // stopImpersonatingUser
    /*
     * Post contains new Company info. Either brand new or updated (if $updating is true)
     */


    function addOrUpdateCompany($co, $investorID, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to notestable
         */
        $errorFound = "";

        // angeldie("addOrUpdateCompany: (updating,investorID) - post" . angelParens($updating, $investorID) . " - " . serialize($_POST));     // debug
        if (!$updating) {
            $co->creatorID = $investorID;
            $co->kind = "startup";
            $co->status = "active";
        }

        /*
         * If the companyName is numeric it HAS to be the id of the company we are updating. Else it should
         * never happen (jscript should disallow selecting a new company)
         */
        $companyName = $_POST['companyName'];
        if (is_numeric($companyName)) {
            if ($co->id != $companyName)
                angeldie("addOrUpdateCompany: should never be able to update to another company!");
        } else
            $co->name = $companyName;
        $co->other_names = $_POST['companyOtherNames'];
        $co->shortdesc = $_POST['companyShortdesc'];
        $co->description = $_POST['companyDescription'];
        $co->url = $_POST['companyURL'];
        $co->sector = $_POST['companySector'];
        if (array_key_exists("YCbatch", $_POST)) {
            $season = $_POST["YCseason"];
            $year = $_POST["YCyear"];
            $co->YCbatch = substr($season, 0, 1) . $year;
            // angeldie($co->YCbatch);  // debug
        } else
            $co->YCbatch = "";

        $conn = $co->connect();

        $fileName = $_FILES['logo']['name'];

        $id = $co->id;
//        $co->status = $_POST['status'];


        if ($co->sector != "" and !is_numeric($co->sector)) {
            /*
             * this is a new sector
             *
             */
            $sector = mysqli_escape_string($conn, $co->sector );
            $se = new angeldb();
            $q = "INSERT INTO sectortable (name) VALUES ('$sector')";
            $result = mysqli_query($conn,$q);
            if ($result == NULL)
                angeldie("sectortable insert of new sector $sector failed:" . $q );

        } else {
            /*
             * co->sector is the id of the sector we want - replace it with the actual sector name:
             * we decided the indirection was just not worth the trouble.
             */
            if ($co->sector != "") {
                $r = $co->getRows("sectorTable", "id=$co->sector");
                if ($r == null)
                    angeldie("failed accessing sector with id: $co->sector");
                $row = mysqli_fetch_row($r);
                if ($row == null)
                    angeldie("sector with id: $co->sector does not exist");
                $co->sector = $row[1];
            }
        }


        if ($fileName != "") {
            /*
             * Really need to do a lot of error checking here. For now, just move the file and assume all is ok.
             * Also, it is a big ugly to assume we are one directory below the img directory, but this seems
             * easiest for now...probably should do this off wherever the "document" root is ('/img' does not work).
             */

            $n = str_replace(' ', '', $fileName);  // strip out whitespace
            $imageType = angelImageType($fileName);

            if ($imageType == "") {
                $errorFound = "Illegal image format: only jpegs and pngs are supported.";
            }


            if ($errorFound == "") {
                $destFilename = sprintf("img/logos/%s_%s.%s", $n, $id, $imageType);
                $thumbFilename = sprintf("img/logos/%s_%s-thumb.%s", $n, $id, $imageType);
                if (!move_uploaded_file($_FILES['logo']['tmp_name'], $destFilename)) {
                    angeldie("move file failed: " . print_r($_FILES, true));
                }
                // $logo = sprintf("/img/logos/%s/%s_%s.jpg", $cohort, urlencode($n), $id);
                $co->logo = sprintf("img/logos/%s_%s-thumb.%s", urlencode($n), $id,$imageType);
                angelMakeThumb($destFilename, $thumbFilename, 400);   // arbitrarily choosing 400 in the hopes that it will look decent
            }
        }

        if ($errorFound == "") {


            /*
             * no error checking??? fix this!
            */
            if ($updating) {
                $id = $co->updateThisRow();
            } else {
                $id = $co->insertRow();
                /*
                 * add investment to company as well.
                 */
            }

            // angeldie($q);    // debug

        }

        return($errorFound);

    }   // addOrUpdateCompany

    /*
     * Post contains new tracker info for investor ($id). Either brand new or updated (if $updating is true)
     */


    function addOrUpdateTracker($id, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to investmenttable
         */
        $errorFound = "";

        $tr = new angelTracker();
        $conn = $tr->connect();

        $tr->investorID = $id;
        $tr->type = $_POST['type'];         // person or company
        $tr->frequency = $_POST['frequency'];
        $tr->start = $_POST['start'];

        if (!$updating) {
            $tr->trackerDate = date('Y-m-d');
            $tr->lastStatusUpdate = date('Y-m-d');
            $tr->nextContactDate = $this->calculateTrackerNextContactDate($tr->frequency, $tr->start);
        }


        if ($tr->type == "person") {
            $tr->personID = $_POST['person'];
            $tr->companyID = $_POST['company'];
            if (!is_numeric($tr->personID)) {
                /*
                 * the person is new
                 */
                $personName = mysqli_escape_string($conn, $tr->personID );
                $names = preg_split("/[\s,]+/", $personName, 2);
                $firstname = $names[0];
                $lastname = $names[1];

                $p = new angelPerson();
                $p->firstname = $firstname;
                $p->lastname = $firstname;
                $p->connect();
                $p = $p->findMatchingPerson(false);
                if ($p != NULL) {  // found it!
                    $tr->personID = $p->id;
                } else {
                    $q = "INSERT INTO persontable (kind,firstname,lastname) VALUES ('angel','$firstname','$lastname')";
                    $result = mysqli_query($conn,$q);
                    if ($result == NULL)
                        angeldie("addOrUpdateTracker: persontable insert of new investor $firstname $lastname failed:" . $q );

                    $q = "SELECT id from persontable WHERE firstname='$firstname' AND lastname='$lastname'";
                    $result = mysqli_query($conn,$q);

                    if ($result == NULL)
                        angeldie("addOrUpdateTracker: persontable select for new investor $firstname $lastname failed:" . $q );

                    $p = mysqli_fetch_object($result, 'angelPerson');
                    $tr->personID = $p->id;
                }
            }  else {
                // confirm this is not a dup
                if (!$updating) {
                    $id = $tr->personID;
                    $q = "SELECT id from trackertable WHERE personID=$id";
                    $result = mysqli_query($conn,$q);
                    if (mysqli_num_rows($result) != 0) {
                        $errorFound = "That person is already being tracked.";
                    }
                }
            }

        } else {
            $tr->companyID = $_POST['company'];
            if (!is_numeric($tr->companyID)) {
                /*
                 * the company is new, or not quite an exact match. see if we can find it first
                 */
                $companyName = mysqli_escape_string($conn, $tr->companyID );
                $co = new angelCompany();
                $co->name = $companyName;
                $co->connect();
                $co = $co->findMatchingCompany();
                if ($co != NULL) {  // found it!
                    $tr->companyID = $co->id;
                } else {
                    $q = "INSERT INTO companytable (kind,name) VALUES ('startup','$companyName')";
                    $result = mysqli_query($conn,$q);
                    if ($result == NULL)
                        angeldie("addOrUpdateTracker: companytable insert of new startup $tr->companyID failed:" . $q );

                    $q = "SELECT id from companytable WHERE name='$companyName'";
                    $result = mysqli_query($conn,$q);

                    if ($result == NULL)
                        angeldie("addOrUpdateTracker: companytable select for new company $tr->companyID failed:" . $q );

                    $co = mysqli_fetch_object($result, 'angelCompany');
                    $tr->companyID = $co->id;
                }
            } else {
                // confirm this is not a dup
                if (!$updating) {
                    $id = $tr->companyID;
                    $q = "SELECT id from trackertable WHERE companyID=$id";
                    $result = mysqli_query($conn,$q);
                    if (mysqli_num_rows($result) != 0) {
                        $errorFound = "That company is already being tracked.";
                    }
                }
            }
        }



        if (!$updating) {
            if ($tr->start == "Now") {
                $tr->status = "live";
            } else
                $tr->status = "wait";
        }

        if (!is_numeric($tr->companyID) and !is_numeric($tr->personID) ) {
            $errorFound .= "Problem with this company or person.<cr>";
        }

        $tr->notes = mysqli_escape_string($conn, $_POST['trackerNotes']);


        if ($errorFound == "") {

            if ($updating) {
                $tr->id = $_POST['update'];     // id of payout to update
                $foundTracker = $tr->getEntry();
                $tr->status = $foundTracker->status;
                $tr->trackerDate = $foundTracker->trackerDate;
                $tr->lastStatusUpdate = $foundTracker->lastStatusUpdate;
                $tr->nextContactDate = $this->calculateTrackerNextContactDate($tr->frequency, "");
                $id = $tr->updateThisRow();
            } else {
                $id = $tr->insertRow();
            }

            // angeldie($q);    // debug

        }

        return($errorFound);
    }   // addOrUpdateTracker

    /*
     * calculate the next tracker date contact. If $start == "" this is for ongoing tracker, otherwise for a new
     * tracker/
     */
    function calculateTrackerNextContactDate($freq, $start) {
        $nextContactDate = "";

        if ($start == "Now") {
            /* next date will be one period from today */
            if ($freq == "monthly") {
                $nextContactDate = date('Y-m-d', strtotime("+1 month"));
            } elseif ($freq == "quarterly") {
                $nextContactDate = date('Y-m-d', strtotime("+3 months"));

            } elseif ($freq == "yearly") {
                $nextContactDate = date('Y-m-d', strtotime("+1 year"));
            } else
                $nextContactDate = $nextContactDate = date('Y-m-d', strtotime("+1 month")); // should never happen

        } elseif ($start == "Next Month") {
            /* next date is one month from now */
            $nextContactDate = date('Y-m-d', strtotime("+1 month"));
        } elseif ($start == "Next Quarter") {
            /* next date is one quarter from now */
            $nextContactDate = date('Y-m-d', strtotime("+3 months"));
        } elseif ($start == "") {
            /* next date will be one period from today */
            if ($freq == "monthly") {
                $nextContactDate = date('Y-m-d', strtotime("+1 month"));
            } elseif ($freq == "quarterly") {
                $nextContactDate = date('Y-m-d', strtotime("+3 months"));

            } elseif ($freq == "yearly") {
                $nextContactDate = date('Y-m-d', strtotime("+1 year"));
            } else
                $nextContactDate = $nextContactDate = date('Y-m-d', strtotime("+1 month")); // should never happen
        }


    return($nextContactDate);
    }
    /*
     * Called from javascript to delete a tracker
     */
    function ajaxDeleteTracker($tr, $investorID) {

        $tr->deleteRows("trackertable","id=$tr->id");

    }

    /*
     * Called from javascript to update the status of a tracker
     */
    function ajaxUpdateTrackerStatus($tr, $investorID, $status) {

        $tr->status = $status;
        $today = date('Y-m-d');
        $id = $tr->id;

        $tr->updateRow("lastStatusUpdate='$today',status='$status'", "id=$id");

    }

    /*
     * Called from javascript to update the next contact date of a tracker
     */
    function ajaxUpdateTrackerNext($tr, $investorID, $next, $makeWait) {

        // error_log("ajaxUpdateTrackerNext: " . serialize($tr));

        $tr->nextContactDate = $next;
        $updateString = "nextContactDate='$next'";
        if ($makeWait) {
            if ($tr->status != "wait") {
                $tr->status = "wait";
                $today = date("Y-m-d");
                $updateString .= ",status='wait',lastStatusUpdate='$today'";
            }
        }
        $id = $tr->id;

        $tr->updateRow($updateString, "id=$id");

    }


    /*
     * Called from javascript to add a note with a new price prediction
     */
    function ajaxAddPredictionNote($investorID, $co, $price) {

        $note = new angelNote();

        $note->investorID = $investorID;
        $note->companyID = $co->id;
        $note->type = "exit prediction";
        $note->noteDate = date('Y-m-d');
        $note->sharePrice = $price;
        $note->notes = "prediction entered from UI";

        $note->connect();
        $note->insertRow();

    }

 /*
  * Called from javascript to add a note with a new valuation update (in _GET)
  */
    function ajaxAddValuationUpdateNote($investorID, $co) {

        $note = new angelNote();

        $note->investorID = $investorID;
        $note->companyID = $co->id;
        $note->type = "valuation update";
        $note->noteDate = date('Y-m-d');
        $note->sharePrice = $_GET["price"];
        $note->valuation = $_GET["valuation"];
        $note->invested = $_GET["invested"];
        $note->FDshares = $_GET["FDshares"];
        $note->round = $_GET["round"];
        $note->notes = "valuation update entered from UI";

        $note->connect();
        $note->insertRow();

    }

/*
 * Called from javascript to add a note with a new fund valuation update (in _GET) -- either a value update or prediction
 */
    function ajaxAddFundValuationUpdateNote($investorID, $fund) {

        $note = new angelFundNote();

        $note->investorID = $investorID;
        $note->fundID = $fund->id;

        $note->type = $_GET["fundUpdateType"];
        $note->noteDate = date('Y-m-d');
        if ($note->type == "valuation update")
            $note->capitalAccount = $_GET["valuation"];
        else
            $note->predictedReturn = $_GET["valuation"];

        $note->notes = $note->type . " entered from UI";

        $conn = $note->connect();
        $note->insertRow();

        $fund->shareConn($conn);

        $this->updateFundModel($fund,$_GET["model"], $investorID);
    }

    function ajaxFundNote($foundFund, $investorID, $noteID) {
        $note = new angelFundNote();
        $note->id = $noteID;
        $conn = $note->connect();
        $noteInput = $note->getEntry();    // get entry of the requested note
        if ($noteInput == 0) {
            echo("AC-ERR='No Such Note ID'");
            die();
        } else if ($noteInput->investorID != $investorID and $investorID != 0) {
            echo("AC-ERR='You cannot edit this note'");
            die();
        }

        $queryString = http_build_query($noteInput);
        echo($queryString);

    }    // ajaxFundNote

    function updateFundModel($fund, $model, $investorID) {
        // angeldie("updateFund: model: " . $model);
        $mod = $fund->getModel();
        if ($mod == "") {     // no model specified, add one
            if ($model != "standard")
                $fund->addModel($model, $investorID);
        } else {
            if ($model == "standard")           // reverting to standard model, remove whatever was there
                $fund->deleteModel();
            else
                $fund->updateModel($model);     // updating to a new model
        }


    }

 /*
  * Post contains new Note info for investor ($id). Either brand new or updated (if $updating is true)
  */


    function addOrUpdateCompanyNote($id, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to notestable
         */
        $errorFound = "";

        $note = new angelNote();
        $conn = $note->connect();

        $note->investorID = $id;
        $note->companyID = $_POST['id'];
        $note->type = $_POST['type'];     // assume good since a selector
        $note->noteDate = $_POST['date'];     // date selector
        if (array_key_exists("public", $_POST))
            $note->public = true;
        else
            $note->public = false;

        if ($note->noteDate == "") {
            $errorFound .= "Please enter a date.<cr>";
        } else {
            //angeldie($inv->inv_date);
            $note->noteDate = date('Y-m-d', strtotime(str_replace('-', '/', $note->noteDate)));
        }

        $note->notes = mysqli_escape_string($conn, $_POST['companyNote']);

        if (array_key_exists("companyValuation", $_POST))
            $note->valuation = $_POST["companyValuation"];
        else
            $note->valuation = "";

        if (array_key_exists("companyFDshares", $_POST))
            $note->FDshares = $_POST["companyFDshares"];
        else
            $note->FDshares = "";

        if (array_key_exists("companySharePrice", $_POST))
            $note->sharePrice = $_POST["companySharePrice"];
        else
            $note->sharePrice = "";

        if (array_key_exists("companyExitSharePrice", $_POST) and $note->type == "exit prediction")
            $note->sharePrice = $_POST["companyExitSharePrice"];

        if (array_key_exists("companyInvested", $_POST))
            $note->invested = $_POST["companyInvested"];
        else
            $note->invested = "";

        if (array_key_exists("companyRound", $_POST)) {
            /*
             * may be a new round or a round id which we need to convert to a round name
             */
            $note->round = angelRound($_POST["companyRound"], $conn);

        } else
            $note->round = "";

        if (array_key_exists("companyStockSplitRatio", $_POST)) {
            /*
             * there has been a stock split
             */
            $note->splitRatio = $_POST["companyStockSplitRatio"];

        } else
            $note->splitRatio = 0;


        if ($errorFound == "") {


            if ($updating) {
                $note->id = $_POST['update'];     // id of investment to update
                $id = $note->updateThisRow();
            } else {
                $id = $note->insertRow();
                /*
                 * add investment to company as well.
                 */
            }

            // angeldie($q);    // debug

        }
        return($errorFound);
    }   // addOrUpdateCompanyNote

   function ajaxNote($foundCompany, $investorID, $noteID) {
       $note = new angelNote();
       $note->id = $noteID;
       $conn = $note->connect();
       $noteInput = $note->getEntry();    // get entry of the requested note
       if ($noteInput == 0) {
           echo("AC-ERR='No Such Note ID'");
           die();
       } else if ($noteInput->investorID != $investorID and $investorID != 0) {
           echo("AC-ERR='You cannot edit this note'");
           die();
       }

       $queryString = http_build_query($noteInput);
       echo($queryString);

   }    // ajaxNote

    function ajaxCompany($co) {
        $queryString = http_build_query($co);
        echo($queryString);

    }    // ajaxCompany

    function deleteNote($foundCompany, $investorID, $noteID) {
        $note = new angelNote();
        $note->id = $noteID;
        $conn = $note->connect();
        $noteInput = $note->getEntry();    // get entry of the note to delete
        if ($noteInput == 0) {
            angelerror("No Such Note ID");
            die();
        } else if ($noteInput->investorID != $investorID and $investorID != 0) {
            angelerror("You cannot delete this note");
            die();
        }

        $noteInput->deleteRows("notestable","id=$noteID");


    }    // deleteNote

/*
 * Fund methods for angelprofile controller!
 */
/*
* Post contains new Note info for investor ($id). Either brand new or updated (if $updating is true)
*/

    function addOrUpdateFundNote($id, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to notestable
         */
        $errorFound = "";

        $note = new angelFundNote();
        $conn = $note->connect();

        $note->investorID = $id;
        $note->fundID = $_POST['id'];
        $note->type = $_POST['type'];     // assume good since a selector
        $note->noteDate = $_POST['date'];     // date selector
        if (array_key_exists("public", $_POST))
            $note->public = true;
        else
            $note->public = false;

        if ($note->noteDate == "") {
            $errorFound .= "Please enter a date.<cr>";
        } else {
            //angeldie($inv->inv_date);
            $note->noteDate = date('Y-m-d', strtotime(str_replace('-', '/', $note->noteDate)));
        }

        $note->notes = mysqli_escape_string($conn, $_POST['fundNote']);

        if (array_key_exists("fundCapitalAccount", $_POST))
            $note->capitalAccount = $_POST["fundCapitalAccount"];
        else
            $note->capitalAccount = "";

        if (array_key_exists("fundPredictedReturn", $_POST))
            $note->predictedReturn = $_POST["fundPredictedReturn"];
        else
            $note->predictedReturn = "";


        if ($errorFound == "") {


            if ($updating) {
                $note->id = $_POST['update'];     // id of note to update
                $id = $note->updateThisRow();
            } else {
                $id = $note->insertRow();
                /*
                 * add investment to company as well.
                 */
            }

            // angeldie($q);    // debug

        }
        return($errorFound);
    }   // addOrUpdateFundNote

    /*
* Post contains updated Fund info. Either brand new (not supported in UI just now) or updated (if $updating is true)
*/


    function addOrUpdateFund($fund, $investorID, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to notestable
         */
        $errorFound = "";

        // angeldie("addOrUpdateFund: (updating,investorID) - post" . angelParens($updating, $investorID) . " - " . serialize($_POST));     // debug
        if (!$updating) {
            $fund->creatorID = $investorID;
            $fund->status = "active";
        }

        /*
         * If the fundName is numeric it HAS to be the id of the fund we are updating. Else it should
         * never happen (jscript should disallow selecting a new fund)
         */
        $fundName = $_POST['fundName'];
        if (is_numeric($fundName)) {
            if ($fund->id != $fundName)
                angeldie("addOrUpdateFund: should never be able to update to another fund!");
        } else
            $fund->name = $fundName;
        $fund->other_names = $_POST['fundOtherNames'];
        $fund->description = $_POST['fundDescription'];
        $fund->inceptionYear = $_POST['inceptionYear'];
        $fund->url = $_POST['fundURL'];

        $fund->wireBank = $_POST['fundWireBank'];
        $fund->wireBankAddress = $_POST['fundWireBankAddress'];
        $fund->wireBankRouting = $_POST['fundWireBankRouting'];
        $fund->wireBankSWIFT = $_POST['fundWireBankSWIFT'];
        $fund->wireBankAcct = $_POST['fundWireBankAcct'];
        $fund->wireAcctName = $_POST['fundWireAcctName'];
        $fund->wireReference = $_POST['fundWireReference'];
        $fund->wireFurtherName = $_POST['fundWireFurtherName'];
        $fund->wireFurtherAcct = $_POST['fundWireFurtherAcct'];


        $conn = $fund->connect();

        $id = $fund->id;


        if ($errorFound == "") {


            /*
             * no error checking??? fix this!
            */
            if ($updating) {
                $id = $fund->updateThisRow();
            } else {
                $id = $fund->insertRow();
                /*
                 * add investment to fund as well.
                 */
            }

            // angeldie($q);    // debug

        }

        return($errorFound);

    }   // addOrUpdateFund

}       // angelprofile class
/*
 * View for angelProfile
 */

class angelProfileView extends angelview {
    public $error;
    public $message;


    function __construct() {

        parent::__construct();

        $this->htmlHeader = "<meta http-equiv='cache-control' content='no-cache'><title>Angelcalc Profile</title>";
        /*$this->js .= "
                    <script src='/jscript/jquery-1.11.1.js'></script>
                    <script src='/jscript/jquery-1.11.1.min.js'></script>
                    <script src='/jscript/select2/select2.js'></script>
                    <script src='/jscript/jquery-ui-1.11.0/jquery-ui.js'></script>";*/

        $this->css .= "
            <link href='../jscript/select2/select2.css' rel='stylesheet'>
            <link href='../jscript/jquery-ui-1.11.0/jquery-ui.css' rel='stylesheet'>";


    }

    function companyPageBody($co, $investorID) {

        // angeldie("companyPageBody: " . serialize($co));

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
         * start with the delete dialogue (see angel.js) for a company note
         */
        $page .= "<div id='dialogDeleteCompanyNote' title='Delete Company Note' style='display:none'>
                                                <form id='' action='' method='post' enctype='multipart/form-data'>
                                                    Permanently delete this note?
                                                </form>
                                        </div>";


        /*
         *  Notify of errors and messages and add company summary on the left
         */
        if ($this->error != "")
            $page .= "<script>$(document).ready( function() { $('#datepickerCompanyNote').datepicker(); \$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $page .= "<script>$(document).ready( function() { $('#datepickerCompanyNote').datepicker(); \$.notify('$this->message', 'success');} );</script>";
        else
            $page .= "<script>$(document).ready( function() { $('#datepickerCompanyNote').datepicker(); } );</script>";


        /*
         * notes on the right
         */
        $notesDiv = "";

        $no = new angelNote();
        $conn = $no->connect();
        $r = $no->getRows("notestable", "companyID=$co->id AND (investorID=$investorID OR public=true) ORDER BY noteDate DESC,last_update DESC");

        /*
         * option to add a new note
         */
        $notesDiv .= "<div class='companyPageNotesDiv'>";
        $notesDiv .= "<a href='javascript:onclick=showCompanyNoteAddForm()'><span style='font-size:14px; font-weight: bold '> + &nbsp </span>Add a new note</a><p></p>";

        $notesDiv .=  "<div id='addCompanyNoteDiv' style='display: none; border: 1px solid blue; margin: 20px 0px 20px 0px; padding: 5px 5px 10px 5px;'>";
        $notesDiv .=  $this->newCompanyNoteForm($investorID, $co->id);
        $notesDiv .= "</div>";

        /*
         * Display the notes for this company
         */

        $notesDiv .=    "<div style='display:table; width:40%; float:left'>";
        $p = new angelPerson();
        $p->shareconn($conn);
        $notesArr = array();
        $splitsArr = array();

        while($n = mysqli_fetch_object($r,"angelNote")){
            $date = date_create($n->noteDate);

            $p->id = $n->investorID;
            $u = $p->findMatchingPerson(false);
            $user = $u->firstname . " " . $u->lastname;
            $outputDate = date_format($date, "m/d/Y");
            if ($n->investorID == $investorID) // if the note is owned by this investor
                $editLink = "<a href='javascript:onclick=editCompanyNote($co->id, $n->id)'>edit</a> &nbsp &nbsp
                             <a href='javascript:onclick=deleteCompanyNote($co->id, $n->id)'>delete</a>";
            else
                $editLink = "";

            if ($n->public)
                $public = " <span style='color:blue'>(public)</span> ";
            else
                $public = "";


            $notes = nl2br(stripcslashes($n->notes));
            //angeldie($notes);
            $notesDiv .=    "<div class='companyPageNoteRow'>
                         <div style='display:table-cell; font-size: 14px; '>$notes</div></div>";
            if ($n->type == "valuation update") {
                $notesArr[] = $n;
                $val = "$" . number_format($n->valuation);
                $shares = number_format($n->FDshares);
                if ($n->round == "" or $n->round == null)
                    $round = "?";
                else
                    $round = $n->round;
                $invested = "$" . number_format($n->invested);
                if ($n->valuation != 0 and $n->FDshares != 0)
                    $sharePrice = "$" . number_format($n->valuation / $n->FDshares, 3);
                else
                    $sharePrice = "$" . number_format($n->sharePrice, 3);
                $notesDiv .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>$invested invested ($round) for a Post Money Valuation of $val. There are $shares Shares Outstanding at a Share Price of $sharePrice</div></div>";
            } else if ($n->type == "exit prediction") {
                $notesArr[] = $n;
                $sharePrice = "$" . number_format($n->sharePrice, 3);
                $notesDiv .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>Predicted Exit at a Share Price of $sharePrice</div></div>";
            } else if ($n->type == "stock split") {
                $splitsArr[] = $n;
                $splitRatio = number_format($n->splitRatio, 1);
                $notesDiv .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>Stock Split with Ratio: $splitRatio to 1</div></div>";
            }
            $notesDiv .=     "
                         <div class='companyPageNoteRow'>
                         <div style='display:table-cell; color:#666666'>$outputDate $public $n->type with $user &nbsp &nbsp $editLink</div></div>

                         <div style='display:table-cell;'>&nbsp</div>

                                    ";
        }
        $notesDiv .=    "</div>";   // table div

        $page .= $this->companyLeftSideSummary($co, $investorID, $splitsArr, true);

        $page .= $notesDiv;
        $page .= $this->companyRightSideGraph($co, $investorID, $notesArr, $splitsArr);

        $page .= "</div>";     // company page

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // clear



        return($page);

    }   // companyPageBody


    /*
     * Create a google line graph of company valuation vs your valuation vs predicted valuation
     */
    function companyRightSideGraph($co, $investorID, $notes, $splits) {

        $noteCount = sizeof($notes);
        if ($noteCount == 0)
            return("");

        $shares = $this->countShares($co, $investorID);

        $rsGraph = "";
        $rsGraph .= "<div id='companyChart' style='width:40%; height:400px;float:left; border: 0px solid black; margin: 0px 0px 0px 10px;'>";

        $rsGraph .= '

                    <script type="text/javascript">';
        $rsGraph .= "
                    google.charts.load('current', {'packages':['line','corechart']});
                    google.charts.setOnLoadCallback(drawChart);

    function drawChart() {

      var data = new google.visualization.DataTable();
      data.addColumn('date', 'Date');
      data.addColumn('number', 'Company Valuation');
      data.addColumn('number', 'Your Value');
      data.addColumn('number', 'Predicted Value');

      data.addRows([
      ";
        /*
         * For each valuation update or prediction, add a column. Go in reverse order because the notes were
         * fetched with the newest note first in the array.
         */
        $currentPred = 0;
        $currentCoVal = 0;
        $currentYourVal = 0;
        for ($i=$noteCount-1; $i >= 0; $i-- ) {
            $n = $notes[$i];
            $splitRatio = $this->calcSplitRatio($n, $splits);
            if ($n->type == "valuation update") {
                $currentCoVal = $n->valuation;
                if ($n->sharePrice == 0 and $n->valuation != 0 and $n->FDshares != 0)
                    $n->sharePrice = $n->valuation / $n->FDshares;
                $currentYourVal = $n->sharePrice * $shares * $splitRatio;
            } else
                $currentPred = $n->sharePrice * $shares * $splitRatio;

            // $rsGraph .= "[new Date('" . $n->noteDate . "')," . $currentCap . "," . $currentPred . "],";
            $rsGraph .= "[new Date('" . $n->noteDate . "')," . $currentCoVal . "," . $currentYourVal . "," . $currentPred . "],";

        }
        // angeldie("fundRightSideGraph - so far:  " . serialize($rsGraph));

        $rsGraph .= "
      ]);

      var options = {
        chart: {
          title: 'Company Valuation vs. Your Value Over Time',
          subtitle: 'in millions of dollars (USD)'
        },
        series: {
          // Gives each series an axis name that matches the Y-axis below.
          0: {axis: 'Company'},
          1: {axis: 'You'},
          2: {axis: 'You'}
        },
        axes: {
          // Adds labels to each axis; they don't have to match the axis names.
          y: {
            Company: {label: 'Company Value'},
            You: {label: 'Your Value'}
          }
        },
        width: 700,
        height: 500
      };

      var chart = new google.charts.Line(document.getElementById('companyChart'));

      chart.draw(data, google.charts.Line.convertOptions(options));
    }
         </script>

                    ";

        $rsGraph .= " </div>";
        return($rsGraph);

    }   //companyRightSideGraph

    /*
     * return the number of pre-split shares the investor has in the input company
     */
    function countShares($co, $investorID) {

        $inv = new angelInvest();
        $conn = $inv->connect();
        $invests = $inv->getInvestments($co->id, $investorID);
        $shares = 0;


        foreach($invests as $i) {
            $shares += $i->shares;
        }


        return($shares);
    }

    /*
     * return the number of pre-split shares the investor has in the input company
     */
    function countSharesPostSplits($co, $investorID, $splits) {

        $inv = new angelInvest();
        $conn = $inv->connect();
        $invests = $inv->getInvestments($co->id, $investorID);
        $splitRatio = 1;
        $shares = 0;

        foreach($splits as $n) {
            $splitRatio *= $n->splitRatio;
        }

        foreach($invests as $i) {
            $shares += $i->shares * $splitRatio;
        }


        return($shares);
    }
    /*
     * return the correct split ratio for the input note
     */
    function calcSplitRatio($note, $splits) {

        $splitRatio = 1;
        $splitCount = sizeof($splits);
        if ($splitCount == 0)
            return(1);

        /*
         * go in reverse order (from earlier to later) since notes listed are latest first
         */
        for ($i=$splitCount-1; $i >= 0; $i-- ) {
            $s = $splits[$i];
            $noteTime = strtotime(str_replace('-', '/', $note->noteDate));
            $splitTime = strtotime(str_replace('-', '/', $s->noteDate));
            if ($noteTime < $splitTime)     // we're done these splits are after this note.
                break;

            $splitRatio *= $s->splitRatio;
        }

        return($splitRatio);
    }
    /*
     * create a left side summary for profile and notes pages. if $edit is true, then on notes page and want to show edit link and founders
     */
    function companyLeftSideSummary($co, $investorID, $splits, $edit) {
        $inv = new angelInvest();
        $conn = $inv->connect();
        $ti = $inv->totalInvestment($co, $investorID);
        $itable = $this->simpleInvestmentsTable($investorID, $co->id);
        $totalInvested = "$" . number_format($ti);

        $shares = number_format($this->countSharesPostSplits($co, $investorID, $splits));

        $po = new angelPayout();
        $po->shareconn($conn);
        $tr = $po->totalReturn($co, $investorID);
        $totalReturned = "$" . number_format($tr);

        $ROI = "$" . number_format($tr - $ti);
        if ($ti == 0)
            $ROIpercent = "N/A";
        else
            $ROIpercent = number_format(100*($tr-$ti)/$ti,0) . "%";

        if ($co->logo == NULL or $co->logo == "")
            $logo = "/img/dummylogo.png";
        else
            $logo     = $co->logo;

        $page = "<div class='companyPageNavDiv'>";

        $page .= "<span style='font-size:16px; font-weight: bold '> $co->name </span>";

        $page .= "<p></p><img alt='logo' height='50' src='$logo' width='180' />";
        $page .= "<p /><span style='font-size:12px; font-weight: lighter '> $co->shortdesc </span>";
        if ($co->url != "")
            $page .= "<p />&nbsp &nbsp<span style='font-size:12px; font-weight: lighter '> <a href='$co->url' target='_blank'>$co->url</a> </span>";

        if ($edit) {
            if (angelHasEditRights($investorID, $co->creatorID))
                $editCo = "<a href='/companyPage?id=$co->id&editCompany'>edit company</a>";
            else {
                $editCo = "<a href='javascript:onclick=companyEditRequest(\"$co->name\")'>Request right to edit</a>";
                //$page .=  "<div id='editCompanyRequestDiv' style='display: none; border: 1px solid blue; margin: 20px 0px 20px 0px; padding: 5px 45px 10px 5px;'>";
                //$page .= $this->editCompanyRequestForm($investorID, $co);
                //$page .= "</div>";
            }

            $page .= "<p></p>$editCo";
        } else {
            /*
             * show link back to notes page
             */
            $page .= "<p></p><a href=/companyPage?id=$co->id>Company Notes</a>";
        }
        $page .= "<p />Company Status: <span style='font-size:12px; font-weight: lighter '> $co->status </span>";

        $bu = $co->basisUsed($investorID);
        $basisUsed = "$" . number_format($bu);
        if ($bu != 0)
            $basisPercent = number_format(100*($bu)/$ti,0) . "%";

        $page .= "<div style='border: 1px solid black; margin: 15px 0px 5px 0px'></div>";
        $page .= "<table>";
        $page .=    "<tr><td>Investments </td></tr>";
        $page .=    "<tr><td colspan='2'>$itable</td></tr>";
        $page .=    "<tr><td colspan='2'><hr></td></tr>";
        $page .=    "<tr><td>Total Invested: </td><td style='text-align:right'>$totalInvested</td></tr>";
        $page .=    "<tr><td>Returned: </td><td style='text-align:right'>$totalReturned</td></tr>";
        if ($tr > 0) {
            $page .=    "<tr><td colspan='2'><hr></td></tr>";
            $page .=    "<tr><td>ROI: </td><td  style='text-align:right'>$ROI</td></tr>";
            $page .=    "<tr><td></td>     <td  style='text-align:right'>$ROIpercent</td></tr>";
            $page .=    "<tr><td colspan='2'><hr></td></tr>";
            if ($bu != 0)
                $page .=    "<tr><td>Basis Used: </td><td>$basisUsed ($basisPercent) -- <a href='/companyPage.php?id=$co->id&clearCompanyBasis'>clear</a></td></tr>";
            else
                $page .=    "<tr><td>Basis Used: </td><td>$basisUsed -- <a href='/companyPage.php?id=$co->id&clearCompanyBasis'>clear</a></td></tr>";
        }
        $page .=    "<tr><td colspan='2'><hr></td></tr>";
        $page .=    "<tr><td>Shares you own(ed): </td><td style='text-align:right'>$shares</td></tr>";

        $page .=  "</table>";


        $founders = $this->companyFounders($co, "simple");
        if ($founders != "") {
            $page .= "<div style='border: 1px solid black; margin: 5px 0px 5px 0px'></div>";
            $page .= "<table>";
            $page .=    "<tr><td>Founders: </td><td></td></tr>";
            $page .= "<tr><td>" . $founders . "</td></tr>";
            $page .=  "</table>";
        }

        $page .= "</div>";
        return($page);
    }
    /*
     * return a simple table of all the investments by the input investor in the input company
     */
    function simpleInvestmentsTable($investorID, $companyID) {

        $itable = "<table class='simpleInvestmentsTable'>";
        $inv = new angelInvest();
        $conn = $inv->connect();
        $invests = $inv->getInvestments($companyID, $investorID);

        if (count($invests) == 0)
            return("");

        //angeldie(serialize($invests));
        foreach ($invests as $i) {
            $amount = "$" . number_format($i->invest_amount);
            $invdate = print_month_day_year($i->inv_date);
            if ($i->type == "equity") {
                $val = sprintf("$%01.1fmm Val", $i->prevaluation/1000000);
            } else {
                $val = sprintf("$%01.0fm Cap", $i->cap/1000000);
            }

            $itable .= "<tr><td style='width:150px'>$invdate ($val)</td><td>$amount</td></tr>";
        }
        $itable .= "</table>";

        return($itable);

    }
   /*
    * return new company note form for input person ($id) and company ($companyID)
    */
    function newCompanyNoteForm($id, $companyID) {

        $invForm = "";
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#companyValuation').number(true);
                $('#companyInvested').number(true);
                $('#companyFDshares').number(true);
                $('#propInvest').number(true);
                $('#preMoneyVal').number(true);
                $('#companySharePrice').number(true,4);
                $('#companyExitSharePrice').number(true,4);
                $('#companyStockSplitRatio').number(true,1);

            });</script>";
        $invForm .= "<form id='addCompanyNote' action='/companyPage.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";

        $invForm .= "<input type=hidden value=$companyID name='id'>";

        $invForm .= '<div class="angelinput">';

        $invForm .= "<table width='100%'><thead>
        <tr>
            <th id='companyNoteTypeHeader' valign=top width='10%'>Type of Note<span style='color:red'>*</span></th>
            <th id='companyNoteDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='companyNoteNoteHeader' valign=top width='10%'>Note<span style='color:red'>*</span></th>
         </tr></thead>";

        $selectedType = 'Meeting';         // default note type is a meeting

        /*
         * Type
         */

        $invForm .= '<tr><td valign=top><select onchange="companyNoteTypeSelector()" id="companyNoteType" name="type">';

        foreach ($GLOBALS["companyNoteTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';
        $invForm .= "<td valign=top><input size='11%' type=text id='datepickerCompanyNote' name='date' value=''></td>";

        $invForm .= "<td valign=top><textarea rows=6 cols=70 id='companyNote' name='companyNote'></textarea></td></tr>";

        $invForm .= "<tr><td colspan='3'><div id='companyPropInvestDiv' style='display:none; width: 500px;'>
                                    Proposed Investment:<input type='text' id='propInvest' name='propInvest' size='15'>
                                    at this pre-money Val:<input type='text' id='preMoneyVal' name='preMoneyVal' size='15'>
                                    </div></td>

                    </tr>";
        $invForm  .= "<tr><td colspan='3'><hr></td></tr>";

        $db = new angeldb();
        $conn = $db->connect();
        $rounds = getAngelRounds($conn);

        $roundScript = "<script>preload_data = 0;";
        $roundScript .= '
        $(document).ready(function () {
            $("#companyRound").select2({
                placeholder: "Series?",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $roundScript .= "data: [";
        foreach (array_keys($rounds) as $id) {
            $round = mysqli_escape_string($conn, $rounds[$id]);
            if ($round == "seed")
                continue;
            $round = ucwords($round);
            if ($needComma)
                $roundScript .= ",";

            $roundScript .= "{id:$id, text:'$round'}";

            $needComma = true;
        }
        $roundScript .= "]";


        $roundScript .= '});

            $("#companyRound").select2("data", preload_data);';
        /*
         * Let's save an ajax call by allowing the Rounds to be easily accessed by id from javascript
         */
        foreach (array_keys($rounds) as $id) {
            $round = mysqli_escape_string($conn, $rounds[$id]);
            $roundScript .= "RoundMap['$round'] = $id;";
        }

        $roundScript .= '});</script>

        <input  id="companyRound" name="companyRound" style="width:200px">';

        $invForm .= "<tr><td colspan='4'><div id='companyExitPredictionDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Exit Price Per Share</th>
                            </tr>
                            <tr>
                                <td><input type='text' id='companyExitSharePrice' name='companyExitSharePrice' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr>";

        $invForm .= "<tr><td colspan='4'><div id='companyValuationDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Round</th>
                                <th>$ Invested</th>
                                <th>Post-Money Valuation</th>
                                <th>Fully-Diluted Shrs</th>
                                <th>Price Per Share</th>
                            </tr>
                            <tr>
                                <td>$roundScript</td>
                                <td><input type='text' id='companyInvested' name='companyInvested' size='12'></td>
                                <td><input type='text' id='companyValuation' name='companyValuation' size='15'></td>
                                <td><input type='text' id='companyFDshares' name='companyFDshares' size='15'></td>
                                <td><input type='text' id='companySharePrice' name='companySharePrice' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr></table>";
        $invForm .= "<tr><td colspan='4'><div id='companyStockSplitDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Stock Split Ratio (2.0 = 2/1 stock split)</th>
                            </tr>
                            <tr>
                                <td><input type='text' id='companyStockSplitRatio' name='companyStockSplitRatio' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr></table>";

        $invForm .= "<input type='hidden' id='companyNoteAction' name='action' value='addCompanyNote'>";
        $invForm .= "<input type='hidden' id='companyNoteUpdate' name='update' value=''>";
        $invForm .= '<p></p> <button type="button" id="newCompanyNoteButton" onclick="addOrUpdateCompanyNote()">Add Note</button>
                     &nbsp &nbsp &nbsp;  <input type="checkbox" id="companyPublic" name="public" value="1">  &nbsp;
                     <span id="companyPublicText">Public? If checked, this note will be visible to other Angelcalc users</span>
                    </form></div>';

        return $invForm;

    }  /* newCompanyNoteForm */

    function companyProfileBody($inputCo, $investorID, $prospect) {

        // angeldie("companyProfileBody: " . serialize($co));

        $name           = htmlentities($inputCo->name, ENT_QUOTES);
        $otherNames     = htmlentities($inputCo->other_names, ENT_QUOTES);
        $shortdesc      = htmlentities($inputCo->shortdesc, ENT_QUOTES);
        $fulldesc       = htmlentities($inputCo->description, ENT_QUOTES);
        $email          = htmlentities($inputCo->email, ENT_QUOTES);
        $url            = htmlentities($inputCo->url, ENT_QUOTES);
        $location       = htmlentities($inputCo->location, ENT_QUOTES);
        $sector         = htmlentities($inputCo->sector, ENT_QUOTES);
        $id             = $inputCo->id;

        $cidTOname = array();

        $inputCo->createMap($cidTOname, "companytable", "id", "name");

        $conn = $inputCo->connect();
        $sectors = getAngelSectors($conn);

        $sectorID = $this->getMySectorID($sector);

        // angeldie(serialize($sectors));       // debug


        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
        * Company summary on the left
        */
        if ($this->error != "")
            $page .= "<script>$(document).ready( function() {\$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $page .= "<script>$(document).ready( function() {\$.notify('$this->message', 'success');} );</script>";

        $page .= $this->companyLeftSideSummary($inputCo, $investorID, false);




        /*
         * Editable stuff on the right
         */


        $page .= "<div class='companyPageNotesDiv'>";

        $page .= "<form id='updateCompany' action='/companyPage?id=$inputCo->id' enctype='multipart/form-data' method='post'>";
        $page .= "<table class=''>";

        /*
         * Company Name
         */

        $page .= "<td id='companyNameLabel'>Name</td><td valign=top>
            <script>";
        if ($id == 0)
            $page .= "preload_data_cos = 0;";
        else {
            $coName = $cidTOname[$id];
            $page .= "preload_data_cos = {id:$id, text:'$coName'};";
        }

        $page .= '
        $(document).ready(function () {
            $("#companyName").select2({
                placeholder: "Select a Company",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $page .= "data: [";
        foreach (array_keys($cidTOname) as $id) {
            $co = mysqli_escape_string($conn, $cidTOname[$id]);
            if ($needComma)
                $page .= ",";

            $page .= "{id:$id, text:'$co'}";

            $needComma = true;
        }
        $page .= "]";


        $page .= '});
            $("#companyName").select2("data", preload_data_cos);';

        $page .= '});
                      </script>';
        if ($prospect) {
            $page .= '<input type="hidden" onchange="loadCompanyProfile(this)" id="companyName" name="companyName" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">Enter or change a name by simply typing it above.</span>
                 </td>';
        } else {
             $page .= '<input type="hidden" onchange="companyMustBeNew()" id="companyName" name="companyName" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">Enter or change a name by simply typing it above.</span>
                 </td>';
        }

        $page .= "<tr><td id='companyShortdescLabel'>Brief Description</td>
                       <td><input style='width:420px' id='companyShortdesc' name='companyShortdesc' type='text' value='$shortdesc'  /></td></tr>";
        $page .= "<tr><td>Full Description</td>
                       <td><textarea rows=6 cols=70 id='companyDescription' name='companyDescription'>$fulldesc</textarea></td></tr>";
        $page .= "<tr><td>Sector</td><script>";
        if ($inputCo->sector == "")
            $page .= "preload_data = 0;";
        else
            $page .= "preload_data = {id:$sectorID, text:'$inputCo->sector'};";
        $page .= '
        $(document).ready(function () {
            $("#e5").select2({
                placeholder: "Sector?",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


                    $needComma = false;

                    $page .= "data: [";
                    foreach (array_keys($sectors) as $id) {
                       $sector = mysqli_escape_string($conn, $sectors[$id]);
                       if ($needComma)
                            $page .= ",";

                       $page .= "{id:$id, text:'$sector'}";

                       $needComma = true;
                    }
                    $page .= "]";


            $page .= '});

            $("#e5").select2("data", preload_data);

            ';

        $page .= '});</script>';




    $page .= '<td><input type="hidden" id="e5" name="companySector" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">For a custom sector just enter its name.</span>
           </td>';



        $page .= "<tr><td>URL</td>
                       <td><input  style='width:420px' id='companyURL' name='companyURL' type='text' value='$url' /></td></tr>";
        $page .= "<tr><td>Logo</td>
                       <td><input style='width:420px' id='companyLogo' name='logo' type='file' /></td>
                       <td><strong>Important!</strong>
                                Please use a large photo (at least 1000px wide) with the logo clearly visible.</td>
                       </tr>";
        $page .= "<tr><td>Other/Former Names</td>
                       <td><input  style='width:420px' id='companyOtherNames' name='companyOtherNames' type='text' value='$otherNames' /></td></tr>";

        $ycYears = $this->selectYCbatchyears($inputCo);
        $summerSelected = "";
        $winterSelected = "";
        if ($inputCo->YCbatch != "") {
            $checked = "checked";
            if (substr($inputCo->YCbatch, 0, 1) == "s")
                $summerSelected  = "selected";
            else
                $winterSelected = "winter";
        } else
            $checked = "";

        $page .= "<tr><td>YC Company?</td>
                       <td><input type='checkbox' name='YCbatch' value='1' $checked><br />
                                                     <div class='exit-reveal-if-active'>

                                                         Batch: &nbsp &nbsp <select name='YCseason'><option $winterSelected >winter</option><option $summerSelected>summer</option></select>
                                                         <select name='YCyear'>$ycYears</select>

                                                         </div></td></tr>
                                                     ";
        if ($prospect) {
            $page .= '<tr><td colspan="2"><hr></td></tr>';
            $page .= "<script type='text/javascript'>$(function() {
                $('#proposedInvestment').number(true);

            });</script>";
            $page .= "<tr><td>Proposed Investment:</td>
                            <td><input style='width:420px' type=text name='proposedInvestment' id='proposedInvestment'></td></tr>
                    ";
        }

        $page .= "</table>";

        $page .= "<div class='companyFounders' style='margin: 20px 10px 40px 0px'>";
        $page .= "<span style='font-size: 16px; color:blue'>Company Founders</span>";
        $page .= "<span style='font-size:14px; font-weight: bold '> &nbsp <a href='javascript:onclick=window.open(\"/userProfile?newFounder&companyID=$inputCo->id\", \"_self\");'>+ Add a founder</a></span><p></p>";
        $page .= $this->companyFounders($inputCo, "full");  // show company founders.

        //$page .= $this->showFounders($inputCo);
        $page .= "</div>";

        /*$page .= "<input type='hidden' id='companyEditAction' name='action' value='updateCompany'>";
         $page .= '<p></p> <button type="button" id="updateCompanyButton" onclick="addOrUpdateCompany()">Update Company</button>
                    </form>';*/
        if  ($prospect) {
            $page .= '<p></p> <button type="button" id="updateCompanyButton" onclick="addOrUpdateCompany()">Create Prospect</button>';
            $page .= "<input type='hidden' id='companyEditAction' name='action' value='createProspect'>";

        } elseif ($inputCo->id == 0) {    // new company
            $page .= '<p></p> <button type="button" id="updateCompanyButton" onclick="addOrUpdateCompany()">Create Company</button>';
            $page .= "<input type='hidden' id='companyEditAction' name='action' value='createCompany'>";
        } elseif (angelHasEditRights($investorID, $inputCo->creatorID)) {
            $page .= '<p></p> <button type="button" id="updateCompanyButton" onclick="addOrUpdateCompany()">Update Company</button>';
            $page .= "<input type='hidden' id='companyID' name='id' value='$inputCo->id'>";
            $page .= "<input type='hidden' id='savedCompanyName' name='savedCompanyName' value='$inputCo->name'>";
            $page .= "<input type='hidden' id='companyEditAction' name='action' value='updateCompany'>";
        } else {
            $page .= "<a href='javascript:onclick=companyEditRequest($inputCo->id, \"$inputCo->name\")'>Request ability to edit this company</a>";
        }

        $page .= "</div>";     // company page

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // clear



        return($page);


    }   // companyProfileBody()

    function getMySectorID($sector) {
        if ($sector == "")
            return(-1);

        $se = new angeldb();
        $conn = $se->connect();
        $r = $se->getRows("sectorTable", "name='$sector'");
        if ($r == null) {
            angeldie(mysqli_error($conn));
            return(-2);
        }
        $row = mysqli_fetch_row($r);
        if ($row == null)
            return(-3);

        return($row[0]);


    }

    function selectYCbatchyears($co) {

        $selectedYear = "";

        if ($co->YCbatch != "") {
            $selectedYear = substr($co->YCbatch, 1, 4);
        }
        $currentYear = date("Y");
        $startYear = 2005;
        $years = "";

        for ($i=$startYear; $i<$currentYear+1; $i++ ) {
            /*
             * either select the batch year of the company or if not in a batch then the current year
             */
            if ($i == $selectedYear)
                $selected = "selected";
            elseif ($selectedYear == "" and $i == $currentYear)
                $selected = "selected";
            else
                $selected = "";

            $years .= "<option $selected>" . $i . "</option>";

        }

        return($years);

    }

    function companyFounders($co, $how) {

        if ($co->founder_ids == null or $co->founder_ids == "")
            return("");

        $conn = $co->connect();
        $founders = explode(",",$co->founder_ids);
        $needOR = false;
        $where = "";
        foreach($founders as $id) {
            if ($needOR)
                $where .= ' OR ';
            else
                $needOR = true;
            $where .= "id=$id";
        }

        $r = $co->getRows("persontable", $where);

        if ($how == "ajax") {
            $rows = array();
            while($row = mysqli_fetch_assoc($r)) {
                $rows[] = $row;
            }
            // angeldie("companyFounders: co.name = " . $co->name . " ajax req: " . json_encode($rows));
            echo json_encode($rows);
            return;
        }

        $foundersDiv = "<div style='display: table; width:100%; border-top: 0px solid gray;padding: 10px 0px 0px 0px'>";

        $foundersDiv .= "<div style='display:table-row'>";
        while($p = mysqli_fetch_object($r,"angelPerson")){
            $firstname = $p->firstname;
            $lastname  = $p->lastname;
            $email     = $p->email;
            $id        = $p->id;

            if ($how == "full") {
                if ($p->photo == NULL)
                    $photo = "/img/founder.jpeg";
                else
                    $photo     = $p->photo;

                $foundersDiv .= "<div style='display:table-cell' onclick='window.open(\"/userProfile?id=$id\", \"_self\");'>

                                <img alt='Thumb' height='100' src='$photo' width='100' />
                ";
            }
            $foundersDiv .= "
            <div class='user_info'>
                <div class='real_name'>
                    <span class='first_name'>$firstname</span>
                    <span class='last_name'>$lastname</span>
                </div>
                <div class='email'>
                    <a href='mailto:$email'>$email</a>
                </div>
            </div>
        </div>
        ";

        }
        $foundersDiv .= "</div></div>";

        return($foundersDiv);

    }   // companyFounders()


    /*
    * return new form to request an edit to a company
    */
    function editCompanyRequestForm($id, $co) {

        $invForm = "";
        $invForm .= "<form id='editCompanyRequestForm' action='/companyPage.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";

        $invForm .= "<input type=hidden value=$co->id name='id'>";

        $invForm .= '<div>';

        $invForm .= "<table width='100%'><thead>
        <tr>
            <th valign=top width='10%'>How would you like <span style='color:blue'>$co->name's </span> profile updated?</th>
         </tr></thead>";

        $invForm .= "<td valign=top><textarea rows=6 cols=30 id='editCompanyRequest' name='editCompanyRequest'></textarea></td></tr></table>";

        $invForm .= '<p></p> <button type="button" class="logsButton" style="font-size: 12px" id="editCompanyRequestButton">Request Change</button>
                    </form></div>';

        return $invForm;

    }  /* newCompanyNoteForm */

    function showFounders($co) {


        return("");

        if ($co->founder_ids == NULL) {
            echo "No Founders!<br>";
            return;
        }
        $conn = $co->returnconn();
        $founders = explode(",",$co->founder_ids);
        $needOR = false;
        $whereList = "";
        foreach($founders as $id) {
            if ($needOR)
                $whereList .= ' OR ';
            else
                $needOR = true;
            $whereList .= "id=$id";
        }

        $q = "SELECT * from persontable WHERE $whereList ORDER BY lastname ASC";
        // echo "query: $q <br>";    // debug

        $r = mysqli_query($conn, $q);
        if (!$r)
            angeldie("bookface select persontable Failed: " . mysqli_error($conn));

        echo "<div style='width:100%; border-top: 2px solid gray'>";

        while($p = mysqli_fetch_object($r,"angelPerson")){
            $firstname = $p->firstname;
            $lastname  = $p->lastname;
            $email     = $p->email;

            if ($p->photo == NULL)
                $photo = "/img/founder.jpeg";
            else
                $photo     = $p->photo;

            echo "
        <div class='userlisting'>
            <img alt='Thumb' height='180' src='$photo' width='180' />
            ";
            /*
            * If this is the logged in user, let them edit their profile
            */
            if ($p->id == $_SESSION["loggedInUserID"] or $_SESSION["auth"] == "root") {
                $id = $p->id;
                echo "
                <div class='edit_user'>
                    <a class='btn btn-xs btn-default' href='/dbm/useredit_person.php?id=$id' style='padding: 3px;'>
                        Edit $firstname's Profile
                    </a>
                </div>
                ";
            }

            echo "
            <div class='user_info'>
                <div class='real_name'>
                    <span class='first_name'>$firstname</span>
                    <span class='last_name'>$lastname</span>
                </div>
                <div class='email'>
                    <a href='mailto:$email'>$email</a>
                </div>
            </div>
        </div>
        ";

        }
    }   // showFounders()

    function userProfilePageBody($investorID, $p, $co) {

        $body = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
         *  Notify of errors and messages and add company summary on the left
         */
        if ($this->error != "")
            $body .= "<script>$(document).ready( function() {\$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $body .= "<script>$(document).ready( function() {\$.notify('$this->message', 'success');} );</script>";

        $body .= $this->personLeftSideSummary($investorID, $p, $co);


        /*
         * Editable stuff on the right
         */

        $cell = angelformatPhoneNumber($p->cell);

        $body .= "<div class='companyPageNotesDiv'>";
        $body .= "<form id='updatePerson' action='/userProfile' enctype='multipart/form-data'  method='post'>";
        $body .= "<table class=''>";

        $body .= "<tr><td id='userFirstNameLabel'>First Name</td>
                       <td><input style='width:420px' id='userFirstName' name='user[firstname]' type='text' value='$p->firstname' /></td></tr>";
        $body .= "<tr><td id='userLastNameLabel'>Last Name</td>
                       <td><input style='width:420px' id='userLastName' name='user[lastname]' type='text' value='$p->lastname' /></td></tr>";
        $body .= "<tr><td id='userEmailLabel'>Email</td>
                       <td><input style='width:420px' id='userEmail' name='user[email]' type='text' value='$p->email' /></td></tr>";
        $body .= "<tr><td id='userLastNameLabel'>Cell Phone</td>
                       <td><input style='width:420px' id='userCell' name='user[cell]' type='text' value='$cell' /></td></tr>";

        if ($p->kind == "user" or $p->kind == "user/founder") {
            $body .= "<tr><td id='userPasswordLabel' colspan='2'>Change Password</td>
                      <tr><td style='text-align:right'>Old</td>
                            <td> <input id='userOpass' name='user[old_password]' type='password' value='' ></td></tr>
                      <tr><td style='text-align:right'>New</td>
                            <td><input id='userNpass' name='user[new_password]' type='password' value='' ></td></tr>
                      <tr><td style='text-align:right'>Confirm</td>
                            <td><input id='userCpass' name='user[conf_password]' type='password' value='' ></td></tr>";
        }
        $body .= "<tr><td id='userLastNameLabel'>Photo</td>
                      <td><input id='user_photo' name='photo' type='file' value='$p->photo' /></td>
                      <td><strong>Important!</strong>
        Please use a large (at least 1000px wide) photo with the face clearly visible.</td></tr>";

        $body .= "<tr><td colspan='2'><hr></td>";
        $body .= "<tr><td id='userBioLabel'>Brief Bio</td>
                       <td><textarea rows='3' cols='70'  id='userBio' name='user[bio]'>$p->bio</textarea></td></tr>";
        $body .= "<tr><td id='userGithubLabel'>Github</td>
                       <td><input style='width:420px' id='userGithub' name='user[github]' type='text' value='$p->github' /></td></tr>";
        $body .= "<tr><td id='userLinkedinLabel'>LinkedIn</td>
                       <td><input style='width:420px' id='userLinkedIn' name='user[linkedin]' type='text' value='$p->linkedin' /></td></tr>";
        $body .= "<tr><td id='userTwitterLabel'>Twitter @</td>
                       <td><input style='width:420px' id='userTwitter' name='user[twitter]' type='text' value='$p->twitter' /></td></tr>";
        $body .= "<tr><td id='userFacebookLabel'>Facebook</td>
                       <td><input style='width:420px' id='userGithub' name='user[facebook]' type='text' value='$p->facebook' /></td></tr>";




        $body .= "</table><p></p>";



        if ($p->email == "") {    // new founder
            $body .= '<p></p> <button type="button" class="modelButton" id="updatePersonButton" onclick="addOrUpdatePerson()">Create Profile</button>';
            $body .= "<input type='hidden' id='companyID' name='companyID' value='$p->company_id'>";
            $body .= "<input type='hidden' id='personEditAction' name='action' value='createPerson'>";
        } elseif (angelHasEditRights($investorID, $p->creatorID)) {
            $body .= '<p></p> <button type="button" class="modelButton" id="updatePersonButton" onclick="addOrUpdatePerson()">Update Profile</button>';
            $body .= "<input type='hidden' id='userID' name='id' value='$p->id'>";
            $body .= "<input type='hidden' id='personEditAction' name='action' value='updatePerson'>";
        } else {
            $body .= "<a href='javascript:onclick=personEditRequest($p->id, \"$p->firstname $p->lastname\")'>Request ability to edit this person</a>";
        }

        if ($p->kind == "user" or $p->kind == "user/founder") {
            if (@$_SESSION["auth"] == "root") {
                $onclick = "onclick='location.href=\"/userProfile?impersonate&id=$p->id\"'";
                $body .= "&nbsp &nbsp<button class='modelButton' type='button' id='updatePersonButton' $onclick>Impersonate This Person</button>";
                //$body .= "&nbsp &nbsp<a href='/userProfile?impersonate&id=$p->id' class='modelButton'>Impersonate This Person</a>";
            } elseif (@$_SESSION["impersonating"] == "yes") {
                $onclick = "onclick='location.href=\"/userProfile?stopImpersonating&id=$p->id\"'";
                $body .= "&nbsp &nbsp<button class='modelButton' type='button' id='updatePersonButton' $onclick>Stop Impersonating</button>";
                // $body .= "&nbsp &nbsp<a href='/userProfile?stopImpersonating&id=$p->id' class='modelButton'>Stop Impersonating</a>";
            }
        }

        $body .= "</form></div>";


        return($body);
    }

  /*
  * create a left side summary for profile and notes pages
  */
    function personLeftSideSummary($investorID, $p, $co) {

        if ($p->photo == NULL or $p->photo == "")
            $photo = "/img/founder.jpeg";
        else
            $photo     = $p->photo;


        $page = "<div class='companyPageNavDiv'>";

        if ($p->email == "") {      // This is a new founder
            $page .= "<span style='font-size:16px; font-weight: bold '> New Founder! </span><br />";
        } else {
            $page .= "<span style='font-size:16px; font-weight: bold '> $p->firstname $p->lastname </span><br />";

        }
        if ($p->company_id)
            $page .= " <span style='font-size:12px; font-weight: bold '>Company: <a href='/companyPage?id=$co->id'>$co->name </span></a>";
        else
            $page .= "<span style='font-size:12px; font-weight: bold '> No company specified </span>";

        if ($p->bio) {
            $page .= "<p></p> <span style='font-size:12px; font-weight:100'>$p->bio </span></a>";
        }

        $page .= "<p></p><img alt='Thumb' height='180' src='$photo' width='180' />";



        $page .= "<div style='border: 1px solid black; margin: 10px 0px 5px 0px'></div>";
        $page .= "<table>";
        if ($p->github) {
            $link = angelCreateSmartLink($p->github, "github");
            $page .=    "<tr><td>$link</td></tr>";
        }
        if ($p->linkedin) {
            $link = angelCreateSmartLink($p->linkedin, "linkedin");
            $page .=    "<tr><td>$link </td></tr>";
        }
        if ($p->twitter) {
            $link = angelCreateSmartLink($p->twitter, "twitter");
            $page .=    "<tr><td>$link</td></tr>";
        }
        if ($p->facebook) {
            $link = angelCreateSmartLink($p->facebook, "facebook");
            $page .=    "<tr><td>$link</td></tr>";
        }
        $page .=  "</table>";

        $page .= "</div>";


        return($page);
    }

    /*
     * Funds!
     */



    function fundPageBody($fund, $investorID) {

        // angeldie("fundPageBody: " . serialize($co));

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
         * start with the delete dialogue (see angel.js) for a fund note
         */
        $page .= "<div id='dialogDeleteFundNote' title='Delete Fund Note' style='display:none'>
                                                <form id='' action='' method='post' enctype='multipart/form-data'>
                                                    Permanently delete this note?
                                                </form>
                                        </div>";


        /*
         *  Notify of errors and messages and add Fund summary on the left
         */
        if ($this->error != "")
            $page .= "<script>$(document).ready( function() { $('#datepickerFundNote').datepicker(); \$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $page .= "<script>$(document).ready( function() { $('#datepickerFundNote').datepicker(); \$.notify('$this->message', 'success');} );</script>";
        else
            $page .= "<script>$(document).ready( function() { $('#datepickerFundNote').datepicker(); } );</script>";

        $page .= $this->fundLeftSideSummary($fund, $investorID, true);
        /*
         * notes on the right
         */
        $notesDiv = "";

        $no = new angelFundNote();
        $conn = $no->connect();
        $r = $no->getRows("fundnotestable", "FundID=$fund->id AND (investorID=$investorID OR public=true) ORDER BY noteDate DESC,last_update DESC");

        /*
         * option to add a new note
         */
        $notesDiv .= "<div class='companyPageNotesDiv'>";
        $notesDiv .= "<a href='javascript:onclick=showFundNoteAddForm()'><span style='font-size:14px; font-weight: bold '> + &nbsp </span>Add a new note</a><p></p>";

        $notesDiv .=  "<div id='addFundNoteDiv' style='display: none; border: 1px solid blue; margin: 20px 0px 20px 0px; padding: 5px 5px 10px 5px;'>";
        $notesDiv .=  $this->newFundNoteForm($investorID, $fund->id);
        $notesDiv .= "</div>";

        /*
         * Display the notes for this Fund
         */

        $notesDiv .=    "<div style='display:table; width:40%; float:left'>";
        $p = new angelPerson();
        $p->shareconn($conn);
        $notesArr = array();

        while($n = mysqli_fetch_object($r,"angelFundNote")){

            $date = date_create($n->noteDate);

            $p->id = $n->investorID;
            $u = $p->findMatchingPerson(false);
            $user = $u->firstname . " " . $u->lastname;
            $outputDate = date_format($date, "m/d/Y");
            if ($n->investorID == $investorID) // if the note is owned by this investor
                $editLink = "<a href='javascript:onclick=editFundNote($fund->id, $n->id)'>edit</a> &nbsp &nbsp
                             <a href='javascript:onclick=deleteFundNote($fund->id, $n->id)'>delete</a>";
            else
                $editLink = "";

            if ($n->public)
                $public = " <span style='color:blue'>(public)</span> ";
            else
                $public = "";


            $notes = nl2br(stripcslashes($n->notes));
            //angeldie($notes);
            $notesDiv .=    "<div class='companyPageNoteRow'>
                         <div style='display:table-cell; font-size: 14px; '>$notes</div></div>";
            if ($n->type == "valuation update") {
                $notesArr[] = $n;
                $acct = "$" . number_format($n->capitalAccount);

                $notesDiv .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>Current capital account: $acct</div></div>";
            } else if ($n->type == "exit prediction") {
                $notesArr[] = $n;
                $predict = "$" . number_format($n->predictedReturn);
                $notesDiv .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>Predicted Return: $predict</div></div>";
            }
            $notesDiv .=     "
                         <div class='companyPageNoteRow'>
                         <div style='display:table-cell; color:#666666'>$outputDate $public $n->type with $user &nbsp &nbsp $editLink</div></div>

                         <div style='display:table-cell;'>&nbsp</div>

                                    ";
        }
        $notesDiv .=    "</div>";   // table div

        $page .= $notesDiv;
        $page .= $this->fundRightSideGraph($notesArr);

        $page .= "</div>";     // fund page

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // clear



        return($page);


    }       // fund page body

    /*
     * Create a google line graph of fund capital account vs predicted return
     */
    function fundRightSideGraph($notes) {

        $noteCount = sizeof($notes);
        if ($noteCount == 0)
            return("");

        $rsGraph = "";
        $rsGraph .= "<div id='thischart' style='width:40%; height:400px;float:left; border: 0px solid black'>";

        $rsGraph .= '

                    <script type="text/javascript">';
        $rsGraph .= "
                    google.charts.load('current', {'packages':['line']});
                    google.charts.setOnLoadCallback(drawChart);

    function drawChart() {

      var data = new google.visualization.DataTable();
      data.addColumn('date', 'Date');
      data.addColumn('number', 'Capital Account');
      data.addColumn('number', 'Predicted Return');

      data.addRows([
      ";
        /*
         * For each valuation update or prediction, add a column. Go in reverse order because the notes were
         * fetched with the newest note first in the array.
         */
        $currentPred = 0;
        $currentCap = 0;
        for ($i=$noteCount-1; $i >= 0; $i-- ) {
            $n = $notes[$i];
            if ($n->type == "valuation update")
                $currentCap = $n->capitalAccount;
            else
                $currentPred = $n->predictedReturn;

            $rsGraph .= "[new Date('" . $n->noteDate . "')," . $currentCap . "," . $currentPred . "],";

        }
        // angeldie("fundRightSideGraph - so far:  " . serialize($rsGraph));

$rsGraph .= "
      ]);

      var options = {
        chart: {
          title: 'Capital Account and Predicted Return over Time',
          subtitle: 'in millions of dollars (USD)'
        },
        width: 700,
        height: 500
      };

      var chart = new google.charts.Line(document.getElementById('thischart'));

      chart.draw(data, google.charts.Line.convertOptions(options));
    }
         </script>

                    ";

        $rsGraph .= " </div>";
        return($rsGraph);

    }       // funcRightSideGraph

 /*
  * create a left side summary for fund profile and notes pages. if $edit is true, then on notes page and want to show edit link and founders
  */
    function fundLeftSideSummary($fund, $investorID, $edit) {

        /*
         * get some summary info - total committed, invested, returned.
         */
        $inv = new angelFundInvest();
        $conn = $inv->connect();
        $ti = $inv->totalInvestment($fund, $investorID);;
        $totalInvested = "$" . number_format($ti);
        $inv->fundID = $fund->id;
        $inv->investorID = $investorID;
        $inv = $inv->findMatchingCommit();
        $totalCommitted = "$" . number_format($inv->investAmount);

        /*
         * get the total return from the distributions for that fund.
         */
        $po = new angelPayout();
        $po->shareconn($conn);
        $tr = $po->totalFundReturn($fund, $investorID);
        $totalReturned = "$" . number_format($tr);

        $ROI = "$" . number_format($tr - $ti);
        if ($ti == 0)
            $ROIpercent = "N/A";
        else
            $ROIpercent = number_format(100*($tr-$ti)/$ti,0) . "%";

        $page = "<div class='companyPageNavDiv'>";

        $page .= "<span style='font-size:16px; font-weight: bold '> $fund->name </span>";

        $page .= "<p /><span style='font-size:12px; font-weight: lighter '> $fund->description </span>";
        if ($fund->url != "")
            $page .= "<p />&nbsp &nbsp<span style='font-size:12px; font-weight: lighter '> <a href='$fund->url' target='_blank'>$fund->url</a> </span>";

        if ($edit) {
            if (angelHasEditRights($investorID, $fund->creatorID))
                $editCo = "<a href='/fundPage?id=$fund->id&editFund'>edit fund</a>";
            else {
                $editCo = "<a href='javascript:onclick=fundEditRequest(\"$fund->name\")'>Request right to edit</a>";
            }

            $page .= "<p></p>$editCo";
        } else {
            /*
             * show link back to notes page
             */
            $page .= "<p></p><a href=/fundPage?id=$fund->id>Fund Notes</a>";
        }
        $page .= "<p />Fund Status: <span style='font-size:12px; font-weight: lighter '> $fund->status </span>";


        $page .= "<div style='border: 1px solid black; margin: 15px 0px 5px 0px'></div>";
        $page .= "<table>";
        $page .=    "<tr><td>Total Committed: </td><td style='text-align:right'>$totalCommitted</td></tr>";
        $page .=    "<tr><td>Total Invested: </td><td style='text-align:right'>$totalInvested</td></tr>";
        $page .=    "<tr><td>Returned: </td><td style='text-align:right'>$totalReturned</td></tr>";
        if ($tr > 0) {
            $page .=    "<tr><td colspan='2'><hr></td></tr>";
            $page .=    "<tr><td>ROI: </td><td  style='text-align:right'>$ROI</td></tr>";
            $page .=    "<tr><td></td>     <td  style='text-align:right'>$ROIpercent</td></tr>";

        }

        $page .=  "</table>";


        $page .= "</div>";
        return($page);
    }


    /*
     * return new fund note form for input person ($id) and fund ($fundID)
     */
    function newFundNoteForm($id, $fundID) {

        $invForm = "";
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#fundPredictedReturn').number(true);
                $('#fundCapitalAccount').number(true);
            });</script>";

        $invForm .= "<form id='addFundNote' action='/fundPage.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";

        $invForm .= "<input type=hidden value=$fundID name='id'>";

        $invForm .= '<div class="angelinput">';

        $invForm .= "<table width='100%'><thead>
        <tr>
            <th id='fundNoteTypeHeader' valign=top width='10%'>Type of Note<span style='color:red'>*</span></th>
            <th id='fundNoteDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='fundNoteNoteHeader' valign=top width='10%'>Note<span style='color:red'>*</span></th>
         </tr></thead>";

        $selectedType = 'Meeting';         // default note type is a meeting

        /*
         * Type
         */

        $invForm .= '<tr><td valign=top><select onchange="fundNoteTypeSelector()" id="fundNoteType" name="type">';

        foreach ($GLOBALS["companyNoteTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';
        $invForm .= "<td valign=top><input size='11%' type=text id='datepickerFundNote' name='date' value=''></td>";

        $invForm .= "<td valign=top><textarea rows=6 cols=70 id='fundNote' name='fundNote'></textarea></td></tr>";


        $invForm  .= "<tr><td colspan='3'><hr></td></tr>";

        $invForm .= "<tr><td colspan='4'><div id='fundExitPredictionDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Predicted Return</th>
                            </tr>
                            <tr>
                                <td><input type='text' id='fundPredictedReturn' name='fundPredictedReturn' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr>";

        $invForm .= "<tr><td colspan='4'><div id='fundValuationDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Current Capital Account</th>
                            </tr>
                            <tr>
                                <td><input type='text' id='fundCapitalAccount' name='fundCapitalAccount' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr></table>";

        $invForm .= "<input type='hidden' id='fundNoteAction' name='action' value='addFundNote'>";
        $invForm .= "<input type='hidden' id='fundNoteUpdate' name='update' value=''>";
        $invForm .= '<p></p> <button type="button" id="newFundNoteButton" onclick="addOrUpdateFundNote()">Add Note</button>
                     &nbsp &nbsp &nbsp;  <input type="checkbox" id="fundPublic" name="public" value="1">  &nbsp;
                     <span id="fundPublicText">Public? If checked, this note will be visible to other Angelcalc users</span>
                    </form></div>';

        return $invForm;

    }  /* newFundNoteForm */

    function fundProfileBody($inputFund, $investorID, $prospect) {

        // angeldie("fundProfileBody: " . serialize($co));

        $name           = htmlentities($inputFund->name, ENT_QUOTES);
        $otherNames     = htmlentities($inputFund->other_names, ENT_QUOTES);
        $fulldesc       = htmlentities($inputFund->description, ENT_QUOTES);
        $email          = htmlentities($inputFund->email, ENT_QUOTES);
        $url            = htmlentities($inputFund->url, ENT_QUOTES);
        $inceptionYear  = htmlentities($inputFund->inceptionYear, ENT_QUOTES);
        $wireBank        = htmlentities($inputFund->wireBank, ENT_QUOTES);
        $wireBankAddress = htmlentities($inputFund->wireBankAddress, ENT_QUOTES);
        $wireBankRouting = htmlentities($inputFund->wireBankRouting, ENT_QUOTES);
        $wireBankSWIFT   = htmlentities($inputFund->wireBankSWIFT, ENT_QUOTES);
        $wireBankAcct    = htmlentities($inputFund->wireBankAcct, ENT_QUOTES);
        $wireAcctName    = htmlentities($inputFund->wireAcctName, ENT_QUOTES);
        $wireReference    = htmlentities($inputFund->wireReference, ENT_QUOTES);
        $wireFurtherName = htmlentities($inputFund->wireFurtherName, ENT_QUOTES);
        $wireFurtherAcct = htmlentities($inputFund->wireFurtherAcct, ENT_QUOTES);


        $id             = $inputFund->id;

        $fidTOname = array();

        $inputFund->createMap($fidTOname, "fundtable", "id", "name");

        $conn = $inputFund->connect();

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
        * Fund summary on the left
        */
        if ($this->error != "")
            $page .= "<script>$(document).ready( function() {\$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $page .= "<script>$(document).ready( function() {\$.notify('$this->message', 'success');} );</script>";

        $page .= $this->fundLeftSideSummary($inputFund, $investorID, false);




        /*
         * Editable stuff on the right
         */


        $page .= "<div class='companyPageNotesDiv'>";

        $page .= "<form id='updateFund' action='/fundPage?id=$inputFund->id' enctype='multipart/form-data' method='post'>";
        $page .= "<table class=''>";

        /*
         * Fund Name
         */

        $page .= "<td id='fundNameLabel'>Name</td><td valign=top>
            <script>";
        if ($id == 0)
            $page .= "preload_data_cos = 0;";
        else {
            $fundName = $fidTOname[$id];
            $page .= "preload_data_cos = {id:$id, text:'$fundName'};";
        }

        $page .= '
        $(document).ready(function () {
            $("#fundName").select2({
                placeholder: "Select a Fund",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $page .= "data: [";
        foreach (array_keys($fidTOname) as $id) {
            $co = mysqli_escape_string($conn, $fidTOname[$id]);
            if ($needComma)
                $page .= ",";

            $page .= "{id:$id, text:'$co'}";

            $needComma = true;
        }
        $page .= "]";


        $page .= '});
            $("#fundName").select2("data", preload_data_cos);';

        $page .= '});
                      </script>';

        $page .= '<input type="hidden" onchange="fundMustBeNew()" id="fundName" name="fundName" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">Enter or change a name by simply typing it above.</span>
                 </td>';

        $page .= "<tr><td>Inception Year</td>
                       <td><input  style='width:420px' id='inceptionYear' name='inceptionYear' type='text' value='$inceptionYear' /></td></tr>";

        $page .= "<tr><td  id='fundDescriptionLabel'>Description</td>
                       <td><textarea rows=6 cols=70 id='fundDescription' name='fundDescription'>$fulldesc</textarea></td></tr>";

        $page .= "<tr><td>URL</td>
                       <td><input  style='width:420px' id='fundURL' name='fundURL' type='text' value='$url' /></td></tr>";

        $page .= "<tr><td>Other/Former Names</td>
                       <td><input  style='width:420px' id='fundOtherNames' name='fundOtherNames' type='text' value='$otherNames' /></td></tr>";

        $page .= "<tr><td colspan='2'><hr></td></tr>";

        $page .= "<tr><td>Bank Name for Wires</td>
                       <td><input  style='width:420px' id='fundWireBank' name='fundWireBank' type='text' value='$wireBank' /></td></tr>";
        $page .= "<tr><td>Bank Address for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankAddress' name='fundWireBankAddress' type='text' value='$wireBankAddress' /></td></tr>";
        $page .= "<tr><td>Bank Routing for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankRouting' name='fundWireBankRouting' type='text' value='$wireBankRouting' /></td></tr>";
        $page .= "<tr><td>Bank SWIFT for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankSWIFT' name='fundWireBankSWIFT' type='text' value='$wireBankSWIFT' /></td></tr>";
        $page .= "<tr><td>Bank Account Number for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankAcct' name='fundWireBankAcct' type='text' value='$wireBankAcct' /></td></tr>";
        $page .= "<tr><td>Bank Account Name for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankAcct' name='fundWireAcctName' type='text' value='$wireAcctName' /></td></tr>";
        $page .= "<tr><td>Reference (if any) for Wires</td>
                       <td><input  style='width:420px' id='fundWireReference' name='fundWireReference' type='text' value='$wireReference' /></td></tr>";
        $page .= "<tr><td>For further credit to name</td>
                       <td><input  style='width:420px' id='fundWireFurtherName' name='fundWireFurtherName' type='text' value='$wireFurtherName' /></td></tr>";
        $page .= "<tr><td>For further credit to acct</td>
                       <td><input  style='width:420px' id='fundWireFurtherAcct' name='fundWireFurtherAcct' type='text' value='$wireFurtherAcct' /></td></tr>";



        $page .= "</table>";


        if ($inputFund->id == 0) {    // new fund
            $page .= '<p></p> <button type="button" id="updateFundButton" onclick="addOrUpdateFund()">Create Fund</button>';
            $page .= "<input type='hidden' id='fundEditAction' name='action' value='createFund'>";
        } elseif (angelHasEditRights($investorID, $inputFund->creatorID)) {
            $page .= '<p></p> <button type="button" id="updateFundButton" onclick="addOrUpdateFund()">Update Fund</button>';
            $page .= "<input type='hidden' id='fundID' name='id' value='$inputFund->id'>";
            $page .= "<input type='hidden' id='savedFundName' name='savedFundName' value='$inputFund->name'>";
            $page .= "<input type='hidden' id='fundEditAction' name='action' value='updateFund'>";
        } else {
            $page .= "<a href='javascript:onclick=fundEditRequest($inputFund->id, \"$inputFund->name\")'>Request ability to edit this fund</a>";
        }

        $page .= "</div>";     // fund page

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // clear



        return($page);


    }   // fundProfileBody()


}