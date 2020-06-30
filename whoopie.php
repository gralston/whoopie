<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/14/15
 * Time: 11:00 AM
 */


set_include_path(get_include_path() . ":" . "./includes" );

require_once "whoopieObjects.inc";
require_once "angel_misc.inc";
require_once "whoopie.inc";  // controller and view for Investment


/*
 * This page is called: (* = done, + = in progress)
 *
 */


session_start();
$int = new whoopie();
$int->view->header = "whoopieheader.inc";

if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
    $int->showWhoopieHomePage();
} else {
    if (array_key_exists("ajaxRequest", $_POST) or array_key_exists("ajaxRequest", $_GET)) {
        // Takes raw data from the request
        $json = file_get_contents('php://input');

        // Converts it into a PHP object
        $data = json_decode($json);
        error_log("ajaxRequest:  " . serialize($data));  // debug
        $int->ajaxWhoopieRequest($data);
    } else if (array_key_exists("ajaxNext", $_POST) or array_key_exists("ajaxNext", $_GET)) {
        error_log("ajaxNext:  " . serialize($_GET));  // debug
        $int->ajaxNextWhoopieEvent();
    } else if (array_key_exists("tester", $_GET)) {
        $int->showWhoopieTesterPage();
    }

}

?>

