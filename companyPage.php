<?PHP

set_include_path(get_include_path() . ":" . "./includes" );

require "angelObjects.inc";
require "angelprofile.inc";              // our controller and view classes
require "angellogin.inc";                    // allows us to change passwords
require "angel_misc.inc";                // include misc useful functions. put more stuff here!


/*
* This page is called:
*  1) to display the company  page
*  2) to take the input from that page and update the entry in the companytable (maybe!)
 * 3) to add a new company note
 * 4) to update an existing company note
 * 5) to clear the basis used for a company (mostly for debugging) and redisplay the company page
*/

$auth = angelAuthenticate("founder"); /* check to make sure we are logged in */
$investorID = $_SESSION["loggedInUserID"];

$ap = new angelProfile();

$co = new angelCompany();
$conn = $co->connect();
if (!$conn)
    angeldie('companyPage: Could not connect: ' . $p->error());

$foundCompany = null;

if (!array_key_exists("newCompany", $_GET) and !array_key_exists("newProspect", $_GET)
    and  !(array_key_exists("action", $_POST) and $_POST["action"] == "createCompany")
    and  !(array_key_exists("action", $_POST) and $_POST["action"] == "createProspect")
   ) {
    if (array_key_exists('id', $_GET)) {
        $id = $_GET['id'];
    } elseif (array_key_exists('id', $_POST)) {
        $id = $_POST['id'];
    } else
        angeldie("companyPage: no company input");


    $co->id = $id;

    $foundCompany = $co->findMatchingCompany(true);
    if ($foundCompany == NULL)
        angeldie("companyPage: No company found with id: $id");
    else
        $co->name = $foundCompany->name;
}

if (array_key_exists('action', $_POST)) {

    $action = $_POST['action'];
    if ($action == "addCompanyNote") { // case (3) above
        angellog("add note request: investorID= " . $investorID);
        //angeldie("add: $id/$investorID"); // debug
        //angeldie(serialize($_POST));  // debug
        $ap->error = $ap->addOrUpdateCompanyNote($investorID, 0);                 /* add the row (0 means not updating) */
        $ap->message = "Note added";
        $ap->showCompanyPage($foundCompany, $investorID);
    } elseif ($action == "updateCompanyNote") {  // case (4) above
        //angeldie("update: $id/$investorID"); // debug
        //angeldie(serialize($_POST));  // debug
        $id = $_POST['update'];
        angellog("update note request: investorID= " . $investorID . "note id: " . $id);
        $ap->error = $ap->addOrUpdateCompanyNote($investorID, $id);                 /* update the indicated row */
        $ap->message = "Note updated";
        $ap->showCompanyPage($foundCompany, $investorID);

    } elseif ($action == "updateCompanyProfile") { // case (2) above
        $foundCompany->shareconn($conn);
        //$ap->updateProfile($foundCompany);
        $ap->message = "Company updated";
        $ap->showCompanyPage($foundCompany, $investorID);     // show the page with the update values
    } elseif ($action == 'updateCompany') {
        if (! angelHasEditRights($investorID, $foundCompany->creatorID)) {
            $ap->error = "You may not edit this company";
            $ap->showCompanyPage($foundCompany, $investorID);
        } else {
            $ap->error = $ap->addOrUpdateCompany($foundCompany, $investorID, 1);
            if ($ap->error == "") {
                $ap->message = addslashes("$foundCompany->name updated");
                $ap->showCompanyPage($foundCompany, $investorID);
            } else
                $ap->showCompanyProfile($foundCompany, $investorID, false);        // error: show profile page again
        }

    } elseif ($action == 'createCompany') {

        $ap->error = $ap->addOrUpdateCompany($co, $investorID, false);      // the false indicates not updating!
        if ($ap->error == "") {
            $co->name = $_POST["companyName"];
            $foundCompany = $co->findMatchingCompany();
            $ap->message = addslashes("New company: $foundCompany->name created");
            $ap->showCompanyPage($foundCompany, $investorID);
        }
    } elseif ($action == 'createProspect') {
        $newProspect = $ap->createNewProspect($co, $investorID);    // returns new company!
        // maybe check if null, but then what do we display? hmmm....
        $ap->showCompanyPage($newProspect, $investorID);
    }
} else {
    if (array_key_exists('editCompany', $_GET)) {
        $ap->showCompanyProfile($foundCompany, $investorID, false);
    } elseif (array_key_exists('ajaxNote', $_GET)) {
        $noteID = $_GET["ajaxNote"];
        $ap->ajaxNote($foundCompany, $investorID, $noteID);
    }  elseif (array_key_exists('ajaxCompany', $_GET)) {
        $ap->ajaxCompany($foundCompany);
    } elseif (array_key_exists('deleteCompanyNote', $_GET)) {
        $noteID = $_GET["deleteCompanyNote"];
        $ap->deleteNote($foundCompany, $investorID, $noteID);
        $ap->message = "Note deleted";
        $ap->showCompanyPage($foundCompany, $investorID);

    } elseif (array_key_exists('ajaxFounders', $_GET)) {
        $ap->view->companyFounders($foundCompany, "ajax");
    }  elseif (array_key_exists('ajaxAddPrediction', $_GET)) {
        $price = $_GET["ajaxAddPrediction"];
        $ap->ajaxAddPredictionNote($investorID, $foundCompany, $price);
    } elseif (array_key_exists('ajaxAddValuationUpdate', $_GET)) {
        $ap->ajaxAddValuationUpdateNote($investorID, $foundCompany);
    } elseif (array_key_exists('clearCompanyBasis', $_GET)) {
        $foundCompany->clearBasis($investorID);
        $ap->message = "Basis Cleared!";
        $ap->showCompanyPage($foundCompany, $investorID);
    } elseif (array_key_exists('newCompany', $_GET)) {
        $ap->showCompanyProfile($co, $investorID, false);
    } elseif (array_key_exists('newProspect', $_GET)) {
        $ap->showCompanyProfile($co, $investorID, true);
    }else
        $ap->showCompanyPage($foundCompany, $investorID);     // case (1) above
}

die();

?>

