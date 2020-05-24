
<title>Angelcalc Tables:Models</title>

<?php
/*
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/1/14
 * Time: 12:07 PM
 */

set_include_path(get_include_path() . ":" . "./includes" . ":" . "../includes" );
require "angelObjects.inc";
require "angel_misc.inc";
require "angeltable.inc";   /* view for tables */

angelAuthenticate("root"); /* check to make sure we are logged in */
ik12log("Model table accessed");

$tab = new angeltable();

$tab->showModelTablePage();

?>
