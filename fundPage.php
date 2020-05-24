<?PHP

set_include_path(get_include_path() . ":" . "./includes" );

require "angelObjects.inc";
require "angelprofile.inc";              // our controller and view classes
require "angellogin.inc";                    // allows us to change passwords
require "angel_misc.inc";                // include misc useful functions. put more stuff here!


/*
* This page is called:
*  1) to display the fund  page
*  2) to take the input from that page and update the entry in the fundtable (maybe!)
 * 3) to add a new fund note
 * 4) to update an existing fund note
 * 5) to clear the basis used for a fund (mostly for debugging) and redisplay the fund page
*/

$auth = angelAuthenticate("founder"); /* check to make sure we are logged in */
$investorID = $_SESSION["loggedInUserID"];

$ap = new angelProfile();

$fund = new angelFund();
$conn = $fund->connect();
if (!$conn)
    angeldie('fundPage: Could not connect: ' . $p->error());


if (array_key_exists('id', $_GET)) {
    $id = $_GET['id'];
} elseif (array_key_exists('id', $_POST)) {
    $id = $_POST['id'];
} else
    angeldie("fundPage: no fund input");


$fund->id = $id;

$foundFund = $fund->findMatchingFund(true);
if ($foundFund == NULL)
    angeldie("fundPage: No fund found with id: $id");
else
    $fund->name = $foundFund->name;


if (array_key_exists('action', $_POST)) {

    $action = $_POST['action'];
    if ($action == "addFundNote") { // case (3) above
        angellog("add note request: investorID= " . $investorID);
        //angeldie("add: $id/$investorID"); // debug
        //angeldie(serialize($_POST));  // debug
        $ap->error = $ap->addOrUpdateFundNote($investorID, 0);                 /* add the row (0 means not updating) */
        $ap->message = "Note added";
        $ap->showFundPage($foundFund, $investorID);
    } elseif ($action == "updateFundNote") {  // case (4) above
        //angeldie("update: $id/$investorID"); // debug
        //angeldie(serialize($_POST));  // debug
        $id = $_POST['update'];
        angellog("update note request: investorID= " . $investorID . "note id: " . $id);
        $ap->error = $ap->addOrUpdateFundNote($investorID, $id);                 /* update the indicated row */
        $ap->message = "Note updated";
        $ap->showFundPage($foundFund, $investorID);

    } elseif ($action == "updateFundProfile") { // case (2) above
        $foundFund->shareconn($conn);
        $ap->updateFundProfile($foundFund);
        $ap->message = "Fund updated";
        $ap->showFundPage($foundFund, $investorID);     // show the page with the update values
    } elseif ($action == 'updateFund') {
        if (! angelHasEditRights($investorID, $foundFund->creatorID)) {
            $ap->error = "You may not edit this fund";
            $ap->showFundPage($foundFund, $investorID);
        } else {
            $ap->error = $ap->addOrUpdateFund($foundFund, $investorID, 1);
            if ($ap->error == "") {
                $ap->message = addslashes("$foundFund->name updated");
                $ap->showFundPage($foundFund, $investorID);
            } else
                $ap->showFundProfile($foundFund, $investorID, false);        // error: show profile page again
        }

    } elseif ($action == 'createFund') {

        $ap->error = $ap->addOrUpdateFund($co, $investorID, false);      // the false indicates not updating!
        if ($ap->error == "") {
            $co->name = $_POST["fundName"];
            $foundFund = $co->findMatchingFund();
            $ap->message = addslashes("New fund: $foundFund->name created");
            $ap->showFundPage($foundFund, $investorID);
        }
    }
} else {
    if (array_key_exists('editFund', $_GET)) {
        $ap->showFundProfile($foundFund, $investorID, false);
    } elseif (array_key_exists('ajaxFundNote', $_GET)) {
        $noteID = $_GET["ajaxFundNote"];
        $ap->ajaxFundNote($foundFund, $investorID, $noteID);
    }  elseif (array_key_exists('ajaxFund', $_GET)) {
        $ap->ajaxFund($foundFund);
    } elseif (array_key_exists('deleteFundNote', $_GET)) {
        $noteID = $_GET["deleteFundNote"];
        $ap->deleteFundNote($foundFund, $investorID, $noteID);
        $ap->message = "Note deleted";
        $ap->showFundPage($foundFund, $investorID);

    } elseif (array_key_exists('ajaxFounders', $_GET)) {
        $ap->view->fundFounders($foundFund, "ajax");
    }  elseif (array_key_exists('ajaxAddPrediction', $_GET)) {
        $price = $_GET["ajaxAddPrediction"];
        $ap->ajaxAddPredictionNote($investorID, $foundFund, $price);
    } elseif (array_key_exists('ajaxAddValuationUpdate', $_GET)) {
        $ap->ajaxAddValuationUpdateNote($investorID, $foundFund);
    } elseif (array_key_exists('ajaxAddFundValuationUpdate', $_GET)) {
        $ap->ajaxAddFundValuationUpdateNote($investorID, $foundFund);
    }  elseif (array_key_exists('clearFundBasis', $_GET)) {
        $foundFund->clearBasis($investorID);
        $ap->message = "Basis Cleared!";
        $ap->showFundPage($foundFund, $investorID);
    } elseif (array_key_exists('newFund', $_GET)) {
        $ap->showFundProfile($co, $investorID, false);
    } elseif (array_key_exists('newProspect', $_GET)) {
        $ap->showFundProfile($co, $investorID, true);
    }else
        $ap->showFundPage($foundFund, $investorID);     // case (1) above
}

die();

?>

