

<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 8/26/14
 * Time: 10:37 AM
 */

    set_include_path(get_include_path() . ":" . "./includes" );

    require "angelObjects.inc";
    require "angelstart.inc";              // our controller and view classes
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


    $p->id = $investorID;
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

    $ap->showStartPage($investorID, $foundPerson, $co);


?>