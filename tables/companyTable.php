<title>Angelcalc Tables:Companies</title>

<?php

set_include_path(get_include_path() . ":" . "./includes" . ":" . "../includes" );

require "angelObjects.inc";
require "angel_misc.inc";
require "angeltable.inc";   /* view for tables */

angelAuthenticate("root"); /* check to make sure we are logged in */
angellog("Company table accessed");

$tab = new angeltable();

$tab->showCompanyTablePage();

?>
