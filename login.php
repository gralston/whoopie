<?php
    session_start();

    set_include_path(get_include_path() . ":" . "./includes" );

    // require "member_login.inc";
    require_once "angelObjects.inc";
    require_once "angellogin.inc";   // controller and view for login
    require_once "angel_misc.inc";


    /*
     * This page is called:
     *  1) to display the standard login page
     *  2) if the user successfully enters credentials, redirect them to either a standard home page
     *     (for their auth level) or to the page from which they were redirected to login.
     */

    $fromURI = "";
    if (array_key_exists('from', $_GET)) {
        $fromURI = urlencode($_GET['from']);
        unset($_GET['from']);

    }


    if (array_key_exists("verified", $_GET)) {
        $headerMessage = "Email address verified. Please login here.";
        unset($_GET['verified']);
    } elseif (array_key_exists("passwordreset", $_GET)) {
        $headerMessage = "Your password was successfully reset. Please login here.";
        unset($_GET['passwordreset']);
    }
    else
      $headerMessage = "Welcome!";

    $log = new angellogin();



    if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
        $log->showLoginPage("",$headerMessage,"");
    } elseif (array_key_exists("email", $_POST)) {              /* Case (2) */
        $email = $_POST['email'];
        // angeldie("in login.php: email is: " . $email);
        if (!array_key_exists("password", $_POST)) {
            $log->showLoginPage($email,"","You must enter your password"); // actually can't get here due do js validation
                                                                           // but nothin' wrong with being careful
        } else {
            $password = $_POST['password'];
            $log->loginUser($email,$password,$fromURI);
        }
    }
?>



