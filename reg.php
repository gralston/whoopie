<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/15/14
 * Time: 8:14 PM
 *
 * Registration Page
 */

set_include_path(get_include_path() . ":" . "./includes" );

require "angel_misc.inc";
require "angelObjects.inc";
require "angelreg.inc";      // controller and view for ik12 registration.  Most real work done here.
require "angellogin.inc";    // required for login object to create a password hash.
require "angelinvestments.inc";     // required to save model

    /*
     * This page is called:
     *  1) to display the standard registration page
     *  2) from the form on (1) to create a new registration
     *  3) from the link in a the registration verify email in order to allow the user to access the site.
     *  4) to display the abbreviated reg page (for teachers and maybe others) which just asks for a password
     *     which is called from an email with the key: passonly (and an id)
     *  5) from (4) to create a passonly registration
     */

    $p = new angelPerson();
    $reg = new angelreg();

    // angeldie(serialize($_POST));
    if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
        angellog("Registration page request");
        $reg->showRegPage($p, "");
    } elseif (array_key_exists("email", $_POST)) {              /* Case (2) */

        $p->firstname   = $_POST['firstname'];
        $p->lastname    = $_POST['lastname'];
        $p->email       = $_POST['email'];
        if (array_key_exists('cell', $_POST))
            $p->cell        = $_POST['cell'];

        angellog("Registration request for: " . $p->email . "(" . $p->firstname . " " . $p->lastname . ")");

        if ($reg->saveReg($p)) {
            $reg->sendVerifyEmail($p);
            if (array_key_exists("saveVars", $_POST)){
                parse_str($_POST["saveVars"], $_POST);
                if (array_key_exists("modelName", $_POST)) {    // there really is a model to save
                    $p->findMatchingPerson(true);
                    $ang = new angelinvestments();
                    // angeldie(serialize($_POST));
                    $ang->saveModel($p->id);
                }
            }
        }
    } elseif (array_key_exists("verify", $_GET)) {               /* Case (3) */
        $p->id   = $_GET['id'];
        $p->hash   = $_GET['hash'];
        angellog("Verification request for: " . $p->id . "(" . $p->hash . ")");

        $reg->verifyReg($p);

    } elseif (array_key_exists("passonly", $_GET)) {               /* Case (4) */
        $p->id   = $_GET['id'];
        // $p->hash   = $_GET['hash']; // will there be a hash??

        $reg->showPassonlyPage($p);

    } elseif (array_key_exists("passonly", $_POST)) {               /* Case (5) */
        $p->id   = $_POST['id'];
        $existingPerson = $p->findMatchingPerson(true);
        if ($existingPerson == NULL)
            ik12die("Something is very wrong - illegal user");
        // $p->hash   = $_GET['hash']; // still wondering if there will be a hash I should check
        if ($reg->saveReg($existingPerson)) {
            $reg->sendVerifyEmail($existingPerson);
        }
    } elseif (array_key_exists("saveModel", $_GET)) {
        $reg->showRegPage($p,"Please register to save your model");
    }


function regerror($p, $error) {
    require "reg.inc";
    die("");
}

?>