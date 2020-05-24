<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/14/15
 * Time: 11:00 AM
 */

set_include_path(get_include_path() . ":" . "./includes" );
ini_set("auto_detect_line_endings", true);
// echo ini_get("auto_detect_line_endings");

require_once "angelObjects.inc";
require_once "angel_misc.inc";
require_once "angelinvestments.inc";  // controller and view for Investment

$auth = angelAuthenticate("founder"); /* check to make sure we are logged in */


/*
 * This page is called: (* = done, + = in progress)
 *  1) to display the default Investments page
 *  2) to display the import investment page
 * 2.5) to display the import payout page
 *  3) to display the Investments page with the appropriate filter
 *  4) to add a new Investment into the table and return to the add Investment page
 *  5) to edit an Investment which was previously entered.
 *  6) to update the Investment which was edited.
 *  7) to delete the specified Investment
 * 7.5) to delete the specified Investment on an ajax request. only return error status.
 * 7.6) to delete the specified Payout on an ajax request. only return error status.
 *  8) to import the CSV file specified
 *  9) to display a page to convert the specified SAFE or Note to equity (and to model that conversion)
 * 10) to do the conversion requested from (9)
 * 11) to display the investment performance page
 * 12) go display the conversion page for modeling only
 * 13) AJAX call - model request - return query string to page
 * 14) save a model request
 * 15) to display the payouts (exit) page
 * n)  to display a help page
 *
 */

$int = new angelinvestments();



if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
    $id = $_SESSION["loggedInUserID"];
    angellog("default summary page request - id: " . $id);
    $int->showAngelcalcPage($id, "","");
    // $int->showInvestmentsPage($id);
} else {
    if (array_key_exists("filter", $_GET)) {                /* Case (3) - filter investments not yet implemented */
        $id = $_SESSION["loggedInUserID"];
        angellog("filter investment request: id= " . $id);
        $int->showInvestmentsPage($id);
    } elseif (array_key_exists("display", $_GET)) {
        $id = $_SESSION["loggedInUserID"];
        $disp = $_GET["display"];
        $int->showAngelcalcPage($id, "", $disp);
    } elseif (array_key_exists("action", $_POST)) {            /* Case (4) */
        /*
         * Add or Update a new Investment.
         */
        $action = $_POST["action"];
        if ($action == "add") {
            $investorID = $_SESSION["loggedInUserID"];
            angellog("add investment request: investorID= " . $investorID);
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdateInvestment($investorID, 0);                 /* add the row (0 means not updating) */
            $int->showAngelcalcPage($investorID, "investment added", "investments");
            //$int->showInvestmentsPage($investorID);
        } elseif ($action == "addPayout") {
            // update
            $investorID = $_SESSION["loggedInUserID"];
            angellog("add payout request: investorID= " . $investorID);
            //angeldie("update: $id/$investorID"); // debug
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdatePayout($investorID, 0);                 /* add the row (0 means not updating) */
            $int->showAngelcalcPage($investorID, "distribution added", "payouts");

        } elseif ($action == "addFund") {
            // update
            $investorID = $_SESSION["loggedInUserID"];
            angellog("add fund request: investorID= " . $investorID);
            //angeldie("update: $id/$investorID"); // debug
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdateFundCommit($investorID, 0);                 /* add the row (0 means not updating) */
            $int->showAngelcalcPage($investorID, "Fund added", "funds");

        } elseif ($action == "updateFund") {
            // update
            $id = $_POST['fundUpdate'];
            $investorID = $_SESSION["loggedInUserID"];
            angellog("update fund request: investorID= " . $investorID  . " id=" . $id);
            //angeldie("update: $id/$investorID"); // debug
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdateFundCommit($investorID, $id);                 /* update the row  */
            $int->showAngelcalcPage($investorID, "Fund updated", "funds");

        } elseif ($action == "addFundPayment") {
            // add fund payment
            $investorID = $_SESSION["loggedInUserID"];
            angellog("add fund payment request: investorID= " . $investorID);
            // angeldie("update: $id/$investorID"); // debug
            // angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdateFundPayment($investorID, 0);                 /* add the row (0 means not updating) */
            if (array_key_exists("showWireEmail", $_POST)) {
                $int->showFundPaymentEmail($investorID);
            } else {
                $int->showAngelcalcPage($investorID, "Fund payment added", "fundpayments");
            }

        } elseif ($action == "updateFundPayment") {
        // update fund payment
            $investorID = $_SESSION["loggedInUserID"];
            $id = $_POST['paymentUpdate'];
            angellog("update fund payment request: investorID= " . $investorID);
            //angeldie("update: $id/$investorID"); // debug
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdateFundPayment($investorID, $id);
            $int->showAngelcalcPage($investorID, "Fund payment updated", "fundpayments");

         } elseif ($action == "update") {
            // update
            $id = $_POST['update'];
            $investorID = $_SESSION["loggedInUserID"];
            angellog("update investment request: investorID= " . $investorID . " id=" . $id);
            //angeldie("update: $id/$investorID"); // debug
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdateInvestment($investorID, $id);                 /* add the row (0 means not updating) */
            $int->showAngelcalcPage($investorID, "Investment Updated", "investments");

        } elseif ($action == "updatePayout") {
            // update
            $id = $_POST['update'];
            $investorID = $_SESSION["loggedInUserID"];
            angellog("update payout request: investorID= " . $investorID . " id=" . $id);
            //angeldie("update: $id/$investorID"); // debug
            //angeldie(serialize($_POST));  // debug
            $int->error = $int->addOrUpdatePayout($investorID, $id);                 /* add the row (0 means not updating) */
            $int->showAngelcalcPage($investorID, "Payout Updated", "payouts");
        } else {
            angeldie("investments: unknown action: " . $action);
        }

    }  elseif (array_key_exists("noFeedback", $_GET)) {            /* new CASE! */

        $id = $_SESSION["loggedInUserID"];
        angellog("default investments page w/no feedback request - id: " . $id);
        $int->showAngelcalcPage($id, "","noFeedback");

    } elseif (array_key_exists("readNews", $_GET)) {            /* new CASE! */

        angellog("read news request:  id=" . $id);
        $n = new angelNews();
        $investorID = $_SESSION["loggedInUserID"];
        $n->updateSavedNews($investorID);

    } elseif (array_key_exists("delete", $_GET)) {            /* Case (7.5) */
        $id = $_GET['delete'];

        angellog("ajax delete investment request:  id=" . $id);
        //angeldie("delete: $id");  // debug
        $int->ajaxDeleteInvestment($id);
    } elseif (array_key_exists("fundDelete", $_GET)) {            /* Case ? */
        $id = $_GET['fundDelete'];

        angellog("ajax delete fund investment request:  id=" . $id);
        //angeldie("delete: $id");  // debug
        $int->ajaxDeleteFundInvestment($id);
    } elseif (array_key_exists("deletePayout", $_GET)) {            /* Case (7.6) */
        $id = $_GET['deletePayout'];

        angellog("ajax delete payout request:  id=" . $id);
        //angeldie("delete: $id");  // debug
        $int->ajaxDeletePayout($id);
    } elseif (array_key_exists("delete", $_POST)) {            /* Case (7) */
        $id = $_POST['update'];
        $investorID = $_SESSION["loggedInUserID"];

        angellog("delete investment request: investorID= " . $investorID . " id=" . $id);
        //angeldie("delete: $id");  // debug
        $int->deleteInvestment($id);
        $int->showInvestmentsPage($investorID);
    } elseif (array_key_exists("import", $_GET)) {            /* Case (2) */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("import request: investorID= " . $investorID);
        //angeldie("import: $investorID");
        $int->showImportPage($investorID);

    } elseif (array_key_exists("importFunds", $_GET)) {            /* Case (2) */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("import funds request: investorID= " . $investorID);
        //angeldie("import: $investorID");
        $int->showImportFundsPage($investorID);

    } elseif (array_key_exists("tax", $_GET)) {            /* Case (2) */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("tax request: investorID= " . $investorID);
        //angeldie("tax: $investorID");
        $int->showTaxPage($investorID);

    } elseif (array_key_exists("export", $_GET)) {            /* Case (2) */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("export request: investorID= " . $investorID);
        //angeldie("export: $investorID");
        $int->showExportPage($investorID);

    }  elseif (array_key_exists("importPay", $_GET)) {            /* Deprecated! */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("import payout request: investorID= " . $investorID);
        //angeldie("import: $investorID");
        $int->showImportPayoutPage($investorID);
    } elseif (array_key_exists("csvUpload", $_POST)) {            /* Case (8) */
        $investorID = $_SESSION["loggedInUserID"];
        $type = $_POST['uploadType'];
        angellog("csv upload request: investorID= " . $investorID . "type=" . $type);

        if ($type == "investments") {
            $int->importInvestmentFile($investorID);
        } elseif ($type == "payouts") {
            $int->importPayoutFile($investorID);
        } else if ($type == "ycfile") {
            $int->importYCFile($investorID);
        } else if ($type == "fundinvestments") {
            $int->importfundInvestmentFile($investorID);
        } else if ($type == "fundpayouts") {
            $int->importfundPayoutFile($investorID);
        }
        //angeldie("upload: " . serialize($_POST));

        // $int->showInvestmentsPage($investorID);
    }  elseif (array_key_exists("csvPayoutUpload", $_POST)) {            /* Deprecated! */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("csv payout upload request: investorID= " . $investorID);
        //angeldie("upload: " . serialize($_POST));
        $int->importPayoutFile($investorID);
        // $int->showInvestmentsPage($investorID);
    }  elseif (array_key_exists("convert", $_GET)) {            /* Case (9) */
        $investorID = $_SESSION["loggedInUserID"];
        $investID = $_GET['investID'];
        angellog("convert investment request: investorID= " . $investorID . " investID=" . $investID);
        //angeldie("convert: " . serialize($_GET));
        $int->ajaxConvert($investorID, $investID);
    } elseif (array_key_exists("model", $_GET)) {            /* Case (12) */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("logged in model investment request: investorID= " . $investorID);
        //angeldie("model: " . serialize($_GET));
        $int->showModelHomePage($investorID);
    } elseif (array_key_exists("compare", $_GET)) {            /* Case (12) */

        if ($auth == "guest") {
            // I pretty sure you can never get here / not be logged in for this code
            angellog("guest compare investment request");
            $investorID = 0;
        } else {
            angellog("logged in compare investment request: investorID= " . $investorID);
            $investorID = $_SESSION["loggedInUserID"];
        }

        //angeldie("model: " . serialize($_GET));
        $int->showmodelComparePage($investorID);
    } elseif (array_key_exists("saveConvert", $_POST)) {            /* Case (10) */

        $investorID = $_SESSION["loggedInUserID"];
        $investID = $_POST['investID'];
        angellog("save convert request: investorID= " . $investorID . " investID=" . $investID);
        //angeldie("convertSave: " . serialize($_POST));
        $int->error = $int->convertInvestment($investID);
        $int->showInvestmentsPage($investorID);
    }  elseif (array_key_exists("perf", $_GET)) {            /* Case (11) */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("performance display request: investorID= " . $investorID);
        $int->showPerfPage($investorID);
    } elseif (array_key_exists("payout", $_GET)) {            /* Case (15) */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("payout display request: investorID= " . $investorID);
        $int->showPayoutsPage($investorID);
    } elseif (array_key_exists("saveModel", $_POST)) {            /* Case (14) */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("saveModel request: investorID= " . $investorID);
        //angeldie("model: " . serialize($_POST));
        $newModelID = $int->saveModel($investorID);
        $int->showConversionPage($investorID, 0, $newModelID,"investments","");
    } elseif (array_key_exists("loadModel", $_GET)) {            /* Case (13) */

        $modelID = $_GET["loadModel"];
        angellog("loadModel request: modelID= " . $modelID);
        //angeldie("your model ID is: " . $modelID);
        $int->ajaxModel($modelID, "");
    } elseif (array_key_exists("help", $_GET)) {            /* Case (n) */

        $investorID = $_SESSION["loggedInUserID"];
        $help = $_GET['help'];
        angellog("help investment request: investorID= " . $investorID . " help= " . $help);
        $int->showHelpPage($investorID, $help);
    } elseif (array_key_exists("login", $_GET)) {            /* Case (Z) */
        /*
         * At new login always start with investments page (ignore cookie). However
         * in the future this should be a default view choice - set in profile!!
         */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("login investment request: investorID= " . $investorID);
        $int->showAngelcalcPage($investorID, "","login");

    } elseif (array_key_exists("test", $_GET)) {            /* Case (n) */

        $id = $_SESSION["loggedInUserID"];
        angellog("new angelcalc page " . $id);
        $int->showAngelcalcPage($id, "just testing!","");

    } elseif (array_key_exists("cflow", $_GET)) {            /* Case (n) */

        $id = $_SESSION["loggedInUserID"];
        angellog("cflow angelcalc page " . $id);
        $int->showCashflowPage($id);

    } elseif (array_key_exists("loadEverything", $_GET)) {            /* Case ? */

        if (array_key_exists("friendID", $_GET)) {
            /* need to check if they are friends! */
            $investorID = $_GET["friendID"];
        } else
            $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax load everything request: investorID= " . $investorID);
        $int->ajaxAngelcalcData($investorID);
    } elseif (array_key_exists("loadInvestments", $_GET)) {            /* Case ? */

        if (array_key_exists("friendID", $_GET)) {
            /* need to check if they are friends! */
            $investorID = $_GET["friendID"];
        } else
            $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax investment request: investorID= " . $investorID);
        $int->ajaxInvestments($investorID);
    } elseif (array_key_exists("loadPayouts", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax payout request: investorID= " . $investorID);
        $int->ajaxPayouts($investorID);
    } elseif (array_key_exists("loadValuationNotes", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax valuation note request: investorID= " . $investorID);
        $int->ajaxValuationNotes($investorID);
    } elseif (array_key_exists("loadFundValuationNotes", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax fund valuation note request: investorID= " . $investorID);
        $int->ajaxFundValuationNotes($investorID);
    } elseif (array_key_exists("loadFundModels", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax fund models request: investorID= " . $investorID);
        $int->ajaxFundModels($investorID);
    } elseif (array_key_exists("exit", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax exit: investorID= " . $investorID);
        $int->ajaxExit($investorID);
    } elseif (array_key_exists("loadCompanies", $_GET)) {            /* Case ? */

        if (array_key_exists("friendID", $_GET)) {
            /* need to check if they are friends! */
            $investorID = $_GET["friendID"];
        } else
            $investorID = $_SESSION["loggedInUserID"];

        angellog("ajax companies request: investorID= " . $investorID);
        $int->ajaxCompanies($investorID);
    }  elseif (array_key_exists("loadFunds", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax funds request: investorID= " . $investorID);
        $int->ajaxFunds($investorID);
    } elseif (array_key_exists("loadFundInvestments", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax fund investments request: investorID= " . $investorID);
        $int->ajaxFundInvestments($investorID);
    } elseif (array_key_exists("loadFundPayouts", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax fundpayout request: investorID= " . $investorID);
        $int->ajaxFundPayouts($investorID);
    } elseif (array_key_exists("funds", $_GET)) {            /* XXX Playing with Funds!! */
        $investorID = $_SESSION["loggedInUserID"];
        angellog("default funds page request - id: " . $investorID);
        $int->showFundsPage($investorID, "","");

    }  elseif (array_key_exists("loadModelandInvestCounts", $_GET)) {            /* Case ? */

        $investorID = $_SESSION["loggedInUserID"];
        angellog("ajax load model counts request: investorID= " . $investorID);
        if (isRoot($investorID)) {
            $int->ajaxLoadModelandInvestCounts();
        } else {
            angellog("loadmodelandinvestcounts requested by non-root ID: " . $investorID);
            angeldie("error:not allowed");
        }

    } elseif (empty($_POST)) {
        angellog("error: unknown GET request: " . serialize($_GET));
        angeldie("Unknown GET: " . serialize($_GET));
    } else {
        angellog("error: unknown POST request: " . serialize($_POST));
        angeldie("Unknown POST: " . serialize($_POST));
    }
}
?>
