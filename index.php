<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/14/15
 * Time: 11:00 AM
 */


set_include_path(get_include_path() . ":" . "./includes" );

require_once "angelObjects.inc";
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
}

?>

