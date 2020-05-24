<?PHP

set_include_path(get_include_path() . ":" . "./includes" );

require "angelObjects.inc";
require "angelprofile.inc";              // our controller and view classes
require "angelinvestments.inc";              // our controller and view classes
require "angellogin.inc";                    // allows us to change passwords
require "angel_misc.inc";                // include misc useful functions. put more stuff here!


/*
* This page is called tohandle all tracker requests.
*
*/

$auth = angelAuthenticate("founder"); /* check to make sure we are logged in */
$investorID = $_SESSION["loggedInUserID"];

$ap = new angelProfile();
$int = new angelinvestments();

$tr = new angelTracker();
$conn = $tr->connect();
if (!$conn)
    angeldie('tracker: Could not connect: ' . $p->error());


if (array_key_exists('id', $_GET)) {
    $id = $_GET['id'];
} elseif (array_key_exists('id', $_POST)) {
    $id = $_POST['id'];
} else
    angeldie("tracker: no tracker input");


$tr->id = $id;

$foundTracker = $tr->getEntry();
if ($foundTracker == NULL and !array_key_exists('action', $_POST)) {
    // error_log("tracker: No tracker found with id: $id");
    angeldie("tracker: No tracker found with id: $id");
}



if (array_key_exists('action', $_POST)) {

    $action = $_POST['action'];

    if ($action == "addTracker") {
        // add tracker
        $investorID = $_SESSION["loggedInUserID"];
        angellog("add tracker request: investorID= " . $investorID);
        //angeldie("update: $id/$investorID"); // debug
        //angeldie(serialize($_POST));  // debug
        $int->error = $ap->addOrUpdateTracker($investorID, 0);                 /* add the row (0 means not updating) */
        if ($int->error == "")
            $int->showAngelcalcPage($investorID, "Tracker Added", "tracker");
        else
            $int->showAngelcalcPage($investorID, "", "tracker");

    } elseif ($action == "update") {
        // update tracker
        $id = $_POST['update'];      // id of tracker to update
        $investorID = $_SESSION["loggedInUserID"];
        angellog("update tracker request: investorID= " . $investorID . " id=" . $id);
        //angeldie("update: $id/$investorID"); // debug
        //angeldie(serialize($_POST));  // debug
        $int->error = $ap->addOrUpdateTracker($investorID, $id);                 /* add the row (0 means not updating) */
        $int->showAngelcalcPage($investorID, "Tracker Updated", "tracker");

    } else {
        angellog("tracker: unknown action:" . $action);
        angeldie("tracker: unknown action: " . $action);
    }

} else {
    if (array_key_exists('ajaxTrackerStatusUpdate', $_GET)) {
        $status = $_GET['ajaxTrackerStatusUpdate'];
        $ap->ajaxUpdateTrackerStatus($foundTracker, $investorID, $status);
    } elseif (array_key_exists('ajaxTrackerNextUpdate', $_GET)) {
        $next = $_GET['ajaxTrackerNextUpdate'];
        if (array_key_exists('ajaxTrackerMakeWait', $_GET))
            $makeWait = true;
        else
            $makeWait = false;
        $ap->ajaxUpdateTrackerNext($foundTracker, $investorID, $next, $makeWait);
    }elseif (array_key_exists('ajaxDeleteTracker', $_GET)) {
        // error_log("tracker: id: $foundTracker->id");
        $ap->ajaxDeleteTracker($foundTracker, $investorID);
    }
}

die();

?>

