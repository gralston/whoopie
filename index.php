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
require_once "angelinvestments.inc";  // controller and view for Investment

$loggedIn = angelIsLoggedIn();
if ($loggedIn) {
    $auth = angelAuthenticate("founder"); /* check to make sure we are logged in */
    $GLOBALS["loggedInUserFname"] = $_SESSION["loggedInUserFname"];
} else {
    $auth = "guest";
    $GLOBALS["loggedInUserFname"] = "Guest";
    $_SESSION["loggedInUserFname"] = "Guest";
    $GLOBALS["loggedInUserID"] = "-1";
    $_SESSION["loggedInUserID"] = "-1";
}


/*
 * This page is called: (* = done, + = in progress)

 * 1) go display the conversion page for modeling only
 * 2) AJAX call - model request - return query string to page
 * 3) save a model request
 * 4) display a shared model
 * 5) AJAX call - share a model request. create hash / store and return it
 * m) AJAX feedback
 * n)  to display a help page
 *
 */




$int = new angelinvestments();

if ($auth != "root")
    $int->view->header = "modelheader.inc";

if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
    if ($auth == "guest") {
        angellog("(model.php) default GUEST model page request");
        $investorID = -1;
    } else {
        $investorID = $_SESSION["loggedInUserID"];
        if ($_SESSION["isBetaUser"] == true) {
            angellog("(index.php) beta user logging in:  investorID=" . $investorID);
            header("Location: "  . "/investments");
            die();
        } else
            angellog("(model.php) default logged in model page request:  investorID=" . $investorID);
    }
    $int->showModelHomePage($investorID);
} else {
    if (array_key_exists("saveModel", $_POST)) {            /* Case (3) */

        // if a guest is trying to save a model, kick them to reg, but save the model afterwards.
        if ($auth == "guest") {
            $postVars = http_build_query($_POST);
            header("Location: "  . $GLOBALS["path"] . "/reg?" . $postVars);     // should be a post. (LATER)
            exit();
        } else
            $investorID = $_SESSION["loggedInUserID"];
        //angeldie("model: " . serialize($_POST));
        angellog("(model.php) saveModel request: investorID= " . $investorID);
        $newModelID = $int->saveModel($investorID);
        $int->showConversionPage($investorID, 0, $newModelID, "model", "Model Saved");
    } elseif (array_key_exists("new", $_GET)) {            /* Case (2) */
        if ($auth == "guest") {
            angellog("(model.php) default GUEST model page request");
            $investorID = $GLOBALS["guestID"];
        } else {
            $investorID = $_SESSION["loggedInUserID"];
            angellog("(model.php) default logged in model page request:  investorID=" . $investorID);
        }
        $int->showConversionPage($investorID, 0,0, "model", "");        // new model id of 0 indicates new model

    } elseif (array_key_exists("sample", $_GET)) {            /* Case (2) */
        if ($auth == "guest") {
            angellog("(model.php) default GUEST model page request");
            $investorID = $GLOBALS["guestID"];
        } else {
            $investorID = $_SESSION["loggedInUserID"];
            angellog("(model.php) default logged in model page request:  investorID=" . $investorID);
        }
        $int->showConversionPage($investorID, 0,-1, "model", "");   // new model id of -1 indicates show sample

    } elseif (array_key_exists("loadModel", $_GET)) {            /* Case (2) */

        $modelID = $_GET["loadModel"];
        if (array_key_exists("sharing", $_GET)) {
            $hash = $_GET["sharing"];
        } else
            $hash = "";
        angellog("(model.php) loadModel  request: modelID= " . $modelID);
        //angeldie("your model ID is: " . $modelID);
        $int->ajaxModel($modelID, $hash);
    } elseif (array_key_exists("editModel", $_GET)) {            /* Case (2) */
        if ($auth == "guest") {
            angeldie("sorry - you must be logged in to do this");
        } else
            $investorID = $_SESSION["loggedInUserID"];

        $modelID = $_GET["editModel"];
        angellog("(model.php) editModel  request: modelID= " . $modelID);
        //angeldie("your model ID is: " . $modelID);
        $int->showConversionPage($investorID, 0, $modelID, "model", "");
    } elseif (array_key_exists("deleteModel", $_GET)) {            /* Case (2) */
        if ($auth == "guest") {
            angeldie("sorry - you must be logged in to do this");
        } else
            $investorID = $_SESSION["loggedInUserID"];

        $modelID = $_GET["deleteModel"];
        angellog("(model.php) deleteModel  request: modelID= " . $modelID);
        //angeldie("your model ID is: " . $modelID);
        $int->deleteModel($investorID, $modelID);
        $int->showModelHomePage($investorID);
    } elseif (array_key_exists("dispShare", $_GET)) {            /* Case (4) */

        $hash = $_GET["dispShare"];
        $modelID = $_GET["mod"];
        angellog("(model.php) display shared Model  request: modelID,hash= " . $modelID . "," . $hash);
        //angeldie("your model ID is: " . $modelID);  // debug
        $int->showSharedModelPage($modelID, $hash, "model");
    } elseif (array_key_exists("shareModel", $_GET)) {            /* Case (4) */

        $modelID = $_GET["shareModel"];
        angellog("(model.php) share Model  request: modelID= " . $modelID);
        // angeldie("your model ID is: " . $modelID); // debug
        $int->ajaxShareModel($modelID);
    } elseif (array_key_exists("unShareModel", $_GET)) {            /* Case (4) */

        $modelID = $_GET["unShareModel"];
        angellog("(model.php) unshare Model  request: modelID= " . $modelID);
        // angeldie("your model ID is: " . $modelID); // debug
        $int->ajaxUnShareModel($modelID);
    } elseif (array_key_exists("compare", $_GET)) {            /* Case (12) */
        if ($auth == "guest") {
            angellog("(model.php) default GUEST compare page request");
            $investorID = -1;
        } else {
            $investorID = $_SESSION["loggedInUserID"];
            angellog("(model.php) default logged in compare page request:  investorID=" . $investorID);
        }

        //angeldie("model: " . serialize($_GET));
        $int->showModelComparePage($investorID);
    } elseif (array_key_exists("test", $_GET)) {            /* Case (12) */
        if ($auth == "guest") {
            angellog("(model.php) default GUEST home page request");
            $investorID = -1;
        } else {
            $investorID = $_SESSION["loggedInUserID"];
            angellog("(model.php) default logged in home page request:  investorID=" . $investorID);
        }

        //angeldie("model: " . serialize($_GET));
        $int->showConversionPage2($investorID, 0,0, "model", "");        // new model id of 0 indicates new model
    } elseif (array_key_exists("feedback", $_POST)) {            /* Case (m) */

        if ($auth == "guest")
            $investorID = -1;
        else
            $investorID = $_SESSION["loggedInUserID"];

        angellog("(model.php) feedback: investorID= " . $investorID);

        $int->ajaxFeedback($investorID);

    } elseif (array_key_exists("help", $_GET)) {            /* Case (n) */

        if ($auth == "guest")
            $investorID = -1;
        else
            $investorID = $_SESSION["loggedInUserID"];
        $help = $_GET['help'];
        angellog("(model.php) help  request: investorID= " . $investorID . " help= " . $help);
        $int->showHelpPage($investorID, $help);
    } elseif (empty($_POST)) {
        angellog("(model.php) error: unknown GET request: " . serialize($_GET));
        angeldie("Unknown GET: " . serialize($_GET));
    } else {
        angellog("(model.php) error: unknown POST request: " . serialize($_POST));
        angeldie("Unknown POST: " . serialize($_POST));
    }
}

?>

