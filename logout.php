<html>
    <head>

    <link type="text/css" rel="stylesheet" href="css/ik12.css"/>
    </head>

<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 5/27/14
 * Time: 8:32 AM
 */

set_include_path(get_include_path() . ":" . "./includes" );

require_once "angelObjects.inc";
require_once "angel_misc.inc";
session_start();
$_SESSION['auth'] = "no";
$_SESSION['isAngel'] = false;
$_SESSION["photo"] = "";
$_SESSION["logname"] = "";

$logo = $GLOBALS["logo"];
ik12log("logging out");


?>
<body style="background-color:#F9F9F9;">
<div class="ik12page" style="background-color:#F9F9F9;">

    <br><br>
    <?php
    echo "<a href='index.php'><img src=$logo width='250'></a><br><br>";
    ?>


    <h1>Thanks for visiting! You are now logged out of AngelCalc</h1><br>
    <p> <a href="login">Return to the Login Page</a></p>

</div>
</body>
</html>