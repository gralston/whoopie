<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/2/14
 * Time: 10:21 AM
 */

set_include_path(get_include_path() . ":" . "./includes" );

require "angel_misc.inc";
require "angelObjects.inc";
require "angellogin.inc";            // we require the login to hash the password
require "angelpassrecover.inc";      // controller and view for ik12 passrecover.  Most real work done here.

    /*
     * This page is called:
     *  1) to display the password recovery screen with an email entry field
     *  2) from the form on (1) with the email to look up and send a password recovery email to
     *  3) from the link in the password recovery email in order to display a page in which
     *     a new password is entry
     *  4) from the form in (3) in order to store the new password
     */

    $pr = new angelpassrecover();

    if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
        $pr->showRecoverPage("");
    } elseif (array_key_exists("email", $_POST)) {              /* Case (2) */
        $pr->sendRecoverEmail($_POST["email"]);
    } elseif (array_key_exists("resetlink", $_GET)) {           /* Case (3) */
        $pr->showNewPassPage($_GET["id"],$_GET["hash"]);
    } elseif (array_key_exists("resetpass", $_POST)) {          /* Case (4) */
        $pr->updatePassword($_POST["id"],$_POST["hash"], $_POST["password"]);
    }


?>