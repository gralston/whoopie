

<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 8/26/14
 * Time: 10:37 AM
 */

    set_include_path(get_include_path() . ":" . "./includes" );

    require "angelObjects.inc";
    require "angelprofile.inc";              // our controller and view classes
    require "angellogin.inc";                    // allows us to change passwords
    require "angel_misc.inc";                // include misc useful functions. put more stuff here!


  /*
   * This page is called:
   *  1) to display the user profile page
   *  2) to take the input from that page and update their entry in the persontable.
   *  3) to impersonate a user
   *  4) to stop impersonating a user
   *  5) to display the profile page for a new founder
   *  6) to create a new entry in the person table for a new founder
   */

    $auth = angelAuthenticate("founder"); /* check to make sure we are logged in */
    $investorID = $_SESSION["loggedInUserID"];
    $newFounder = false;

    $ap = new angelProfile();

    /*
     * look up the input person and also grab their company (note that for founders there is no company)
     */
    $p = new angelPerson();
    $conn = $p->connect();
    if (!$conn)
        angeldie('Could not connect: ' . $p->error());


    $action = "";
    if (array_key_exists('action', $_POST)) {
        $action = $_POST['action'];
    } else if (array_key_exists('newFounder', $_GET)) {
        $newFounder = true;
    }


    if ($newFounder or $action == "createPerson") {
        if ($newFounder)
            $companyID = $_GET['companyID'];
        else
            $companyID = $_POST['companyID'];

        $co = new angelCompany();
        $co->shareconn($conn);      // use the existing connection

        $co->id = $companyID;
        if ($co->id != 0)
            $foundco = $co->findMatchingCompany();
        else
            angeldie("creating new founder with no company");

        $co = $foundco;
        $p->company_id = $co->id;

    } else {
        if (array_key_exists('id', $_GET)) {
            $id = $_GET['id'];
        } elseif (array_key_exists('id', $_POST)) {
            $id = $_POST['id'];
        } else
            angeldie("no person input");

        if ($investorID != $id and $_SESSION["auth"] != "root")
            angelerror("You may not edit someone else's profile!");


        $p->id = $id;
        // $cohort = $GLOBALS["cohort"];   // this was a bug, should be the cohort of the person

        $foundPerson = $p->findMatchingPerson(true);
        if ($foundPerson == NULL)
            angeldie("No person found with id: $id");

        $co = new angelCompany();
        $co->shareconn($conn);      // use the existing connection

        $co->id = $foundPerson->company_id;
        if ($co->id != 0)
            $foundco = $co->findMatchingCompany();
        else
            $foundco = NULL;

        if ($foundco == NULL) {
            $co->name = "<i>not specified</i>";
        } else {
            $co->name = $foundco->name;
        }
    }

    if ($newFounder) {
        $ap->showUProfilePage($investorID, $p, $co);     // show the page with the updated values
    } elseif ($action == "updatePerson") {
        $foundPerson->shareconn($conn);
        $ap->addOrUpdateUProfile($foundPerson);            // case (2) above
        $ap->showUProfilePage($investorID, $foundPerson, $co);     // show the page with the updated values
    } elseif ($action == "createPerson") {
        $p->id = $ap->addOrUpdateUProfile($p);                           // case (6) above
        $ap->addFounder($co, $p);
        $ap->showCompanyProfile($co, $investorID);                 // return to the company page (this may suck later if you
                                                                // can add founders from elsewhere... :(
    } elseif (array_key_exists('impersonate', $_GET)) {
        if ($auth != "root")
            angeldie("Only admins may impersonate");
        $foundPerson->shareconn($conn);
        $ap->impersonateUser($foundPerson);                      // case (3) above
        $ap->showUProfilePage($investorID, $foundPerson, $co);
    } elseif (array_key_exists('stopImpersonating', $_GET)) {
        $foundPerson->shareconn($conn);
        $ap->stopImpersonatingUser();                           // case (4) above
        $ap->showUProfilePage($investorID, $foundPerson, $co);
    } elseif (array_key_exists('update', $_POST)) {
        $foundPerson->shareconn($conn);
        $ap->addOrUpdateUProfile($foundPerson);            // case (2) above
        $ap->showUProfilePage($investorID, $foundPerson, $co);     // show the page with the update values
    } else {
        $ap->showUProfilePage($investorID, $foundPerson, $co);     // case (1) above
    }


?>