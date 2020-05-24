<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/8/14
 * Time: 11:04 AM
 */

class angelProfile {

    public $error = "";         // error message to display
    public $message = "";       // (non-error) message to display
    public $view;

    function __construct() {
        $this->view = new angelProfileView();
    }

    /*******************
     * This method is the standard reg page
     */
    function showStartPage($investorID, $p, $co) {

        $this->view->error = $this->error;
        $this->view->message = $this->message;

        $body = $this->view->startPageBody($investorID, $p, $co);
        $this->view->outputHTML($body);
    }



    function addFounder($co, $p) {
        if ($co->founder_ids == "")
            $co->founder_ids = "$p->id";
        else
            $co->founder_ids .= "," . "$p->id";

        $co->updateThisRow();
    }


/*
 * Fund methods for angelprofile controller!
 */
/*
* Post contains new Note info for investor ($id). Either brand new or updated (if $updating is true)
*/

    function addOrUpdateFundNote($id, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to notestable
         */
        $errorFound = "";

        $note = new angelFundNote();
        $conn = $note->connect();

        $note->investorID = $id;
        $note->fundID = $_POST['id'];
        $note->type = $_POST['type'];     // assume good since a selector
        $note->noteDate = $_POST['date'];     // date selector
        if (array_key_exists("public", $_POST))
            $note->public = true;
        else
            $note->public = false;

        if ($note->noteDate == "") {
            $errorFound .= "Please enter a date.<cr>";
        } else {
            //angeldie($inv->inv_date);
            $note->noteDate = date('Y-m-d', strtotime(str_replace('-', '/', $note->noteDate)));
        }

        $note->notes = mysqli_escape_string($conn, $_POST['fundNote']);

        if (array_key_exists("fundCapitalAccount", $_POST))
            $note->capitalAccount = $_POST["fundCapitalAccount"];
        else
            $note->capitalAccount = "";

        if (array_key_exists("fundPredictedReturn", $_POST))
            $note->predictedReturn = $_POST["fundPredictedReturn"];
        else
            $note->predictedReturn = "";


        if ($errorFound == "") {


            if ($updating) {
                $note->id = $_POST['update'];     // id of note to update
                $id = $note->updateThisRow();
            } else {
                $id = $note->insertRow();
                /*
                 * add investment to company as well.
                 */
            }

            // angeldie($q);    // debug

        }
        return($errorFound);
    }   // addOrUpdateFundNote

    /*
* Post contains updated Fund info. Either brand new (not supported in UI just now) or updated (if $updating is true)
*/




}       // angelprofile class
/*
 * View for angelProfile
 */

class angelProfileView extends angelview {
    public $error;
    public $message;


    function __construct() {

        parent::__construct();

        $this->htmlHeader = "<meta http-equiv='cache-control' content='no-cache'><title>Angelcalc Profile</title>";
        /*$this->js .= "
                    <script src='/jscript/jquery-1.11.1.js'></script>
                    <script src='/jscript/jquery-1.11.1.min.js'></script>
                    <script src='/jscript/select2/select2.js'></script>
                    <script src='/jscript/jquery-ui-1.11.0/jquery-ui.js'></script>";*/

        $this->css .= "
            <link href='../jscript/select2/select2.css' rel='stylesheet'>
            <link href='../jscript/jquery-ui-1.11.0/jquery-ui.css' rel='stylesheet'>";


    }



    /*
     * create a left side summary for profile and notes pages. if $edit is true, then on notes page and want to show edit link and founders
     */
    function companyLeftSideSummary($co, $investorID, $edit) {
        $inv = new angelInvest();
        $conn = $inv->connect();
        $ti = $inv->totalInvestment($co, $investorID);
        $itable = $this->simpleInvestmentsTable($investorID, $co->id);
        $totalInvested = "$" . number_format($ti);

        $po = new angelPayout();
        $po->shareconn($conn);
        $tr = $po->totalReturn($co, $investorID);
        $totalReturned = "$" . number_format($tr);

        $ROI = "$" . number_format($tr - $ti);
        if ($ti == 0)
            $ROIpercent = "N/A";
        else
            $ROIpercent = number_format(100*($tr-$ti)/$ti,0) . "%";

        if ($co->logo == NULL or $co->logo == "")
            $logo = "/img/dummylogo.png";
        else
            $logo     = $co->logo;

        $page = "<div class='companyPageNavDiv'>";

        $page .= "<span style='font-size:16px; font-weight: bold '> $co->name </span>";

        $page .= "<p></p><img alt='logo' height='50' src='$logo' width='180' />";
        $page .= "<p /><span style='font-size:12px; font-weight: lighter '> $co->shortdesc </span>";
        if ($co->url != "")
            $page .= "<p />&nbsp &nbsp<span style='font-size:12px; font-weight: lighter '> <a href='$co->url' target='_blank'>$co->url</a> </span>";

        if ($edit) {
            if (angelHasEditRights($investorID, $co->creatorID))
                $editCo = "<a href='/companyPage?id=$co->id&editCompany'>edit company</a>";
            else {
                $editCo = "<a href='javascript:onclick=companyEditRequest(\"$co->name\")'>Request right to edit</a>";
                //$page .=  "<div id='editCompanyRequestDiv' style='display: none; border: 1px solid blue; margin: 20px 0px 20px 0px; padding: 5px 45px 10px 5px;'>";
                //$page .= $this->editCompanyRequestForm($investorID, $co);
                //$page .= "</div>";
            }

            $page .= "<p></p>$editCo";
        } else {
            /*
             * show link back to notes page
             */
            $page .= "<p></p><a href=/companyPage?id=$co->id>Company Notes</a>";
        }
        $page .= "<p />Company Status: <span style='font-size:12px; font-weight: lighter '> $co->status </span>";

        $bu = $co->basisUsed($investorID);
        $basisUsed = "$" . number_format($bu);
        if ($bu != 0)
            $basisPercent = number_format(100*($bu)/$ti,0) . "%";

        $page .= "<div style='border: 1px solid black; margin: 15px 0px 5px 0px'></div>";
        $page .= "<table>";
        $page .=    "<tr><td>Investments </td></tr>";
        $page .=    "<tr><td colspan='2'>$itable</td></tr>";
        $page .=    "<tr><td colspan='2'><hr></td></tr>";
        $page .=    "<tr><td>Total Invested: </td><td style='text-align:right'>$totalInvested</td></tr>";
        $page .=    "<tr><td>Returned: </td><td style='text-align:right'>$totalReturned</td></tr>";
        if ($tr > 0) {
            $page .=    "<tr><td colspan='2'><hr></td></tr>";
            $page .=    "<tr><td>ROI: </td><td  style='text-align:right'>$ROI</td></tr>";
            $page .=    "<tr><td></td>     <td  style='text-align:right'>$ROIpercent</td></tr>";
            $page .=    "<tr><td colspan='2'><hr></td></tr>";
            if ($bu != 0)
                $page .=    "<tr><td>Basis Used: </td><td>$basisUsed ($basisPercent) -- <a href='/companyPage.php?id=$co->id&clearCompanyBasis'>clear</a></td></tr>";
            else
                $page .=    "<tr><td>Basis Used: </td><td>$basisUsed -- <a href='/companyPage.php?id=$co->id&clearCompanyBasis'>clear</a></td></tr>";
        }

        $page .=  "</table>";


        $founders = $this->companyFounders($co, "simple");
        if ($founders != "") {
            $page .= "<div style='border: 1px solid black; margin: 5px 0px 5px 0px'></div>";
            $page .= "<table>";
            $page .=    "<tr><td>Founders: </td><td></td></tr>";
            $page .= "<tr><td>" . $founders . "</td></tr>";
            $page .=  "</table>";
        }

        $page .= "</div>";
        return($page);
    }


    function startPageBody($investorID, $p, $co) {

        $body = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
         *  Notify of errors and messages and add company summary on the left
         */
        if ($this->error != "")
            $body .= "<script>$(document).ready( function() {\$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $body .= "<script>$(document).ready( function() {\$.notify('$this->message', 'success');} );</script>";

        $body .= $this->personLeftSideSummary($investorID, $p, $co);






        return($body);
    }

  /*
  * create a left side summary for profile and notes pages
  */
    function personLeftSideSummary($investorID, $p, $co) {

        if ($p->photo == NULL or $p->photo == "")
            $photo = "/img/founder.jpeg";
        else
            $photo     = $p->photo;


        $page = "<div class='companyPageNavDiv'>";

        if ($p->email == "") {      // This is a new founder
            $page .= "<span style='font-size:16px; font-weight: bold '> New Founder! </span><br />";
        } else {
            $page .= "<span style='font-size:16px; font-weight: bold '> $p->firstname $p->lastname </span><br />";

        }
        if ($p->company_id)
            $page .= " <span style='font-size:12px; font-weight: bold '>Company: <a href='/companyPage?id=$co->id'>$co->name </span></a>";
        else
            $page .= "<span style='font-size:12px; font-weight: bold '> No company specified </span>";

        if ($p->bio) {
            $page .= "<p></p> <span style='font-size:12px; font-weight:100'>$p->bio </span></a>";
        }

        $page .= "<p></p><img alt='Thumb' height='180' src='$photo' width='180' />";



        $page .= "<div style='border: 1px solid black; margin: 10px 0px 5px 0px'></div>";
        $page .= "<table>";
        if ($p->github) {
            $link = angelCreateSmartLink($p->github, "github");
            $page .=    "<tr><td>$link</td></tr>";
        }
        if ($p->linkedin) {
            $link = angelCreateSmartLink($p->linkedin, "linkedin");
            $page .=    "<tr><td>$link </td></tr>";
        }
        if ($p->twitter) {
            $link = angelCreateSmartLink($p->twitter, "twitter");
            $page .=    "<tr><td>$link</td></tr>";
        }
        if ($p->facebook) {
            $link = angelCreateSmartLink($p->facebook, "facebook");
            $page .=    "<tr><td>$link</td></tr>";
        }
        $page .=  "</table>";

        $page .= "</div>";


        return($page);
    }

    /*
     * Funds!
     */



    function fundPageBody($fund, $investorID) {

        // angeldie("fundPageBody: " . serialize($co));

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
         * start with the delete dialogue (see angel.js) for a fund note
         */
        $page .= "<div id='dialogDeleteFundNote' title='Delete Fund Note' style='display:none'>
                                                <form id='' action='' method='post' enctype='multipart/form-data'>
                                                    Permanently delete this note?
                                                </form>
                                        </div>";


        /*
         *  Notify of errors and messages and add Fund summary on the left
         */
        if ($this->error != "")
            $page .= "<script>$(document).ready( function() { $('#datepickerFundNote').datepicker(); \$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $page .= "<script>$(document).ready( function() { $('#datepickerFundNote').datepicker(); \$.notify('$this->message', 'success');} );</script>";
        else
            $page .= "<script>$(document).ready( function() { $('#datepickerFundNote').datepicker(); } );</script>";

        $page .= $this->fundLeftSideSummary($fund, $investorID, true);
        /*
         * notes on the right
         */
        $no = new angelFundNote();
        $conn = $no->connect();
        $r = $no->getRows("fundnotestable", "FundID=$fund->id AND (investorID=$investorID OR public=true) ORDER BY noteDate DESC,last_update DESC");

        /*
         * option to add a new note
         */
        $page .= "<div class='companyPageNotesDiv'>";
        $page .= "<a href='javascript:onclick=showFundNoteAddForm()'><span style='font-size:14px; font-weight: bold '> + &nbsp </span>Add a new note</a><p></p>";

        $page .=  "<div id='addFundNoteDiv' style='display: none; border: 1px solid blue; margin: 20px 0px 20px 0px; padding: 5px 5px 10px 5px;'>";
        $page .=  $this->newFundNoteForm($investorID, $fund->id);
        $page .= "</div>";

        /*
         * Display the notes for this Fund
         */

        $page .=    "<div style='display:table; width:100%'>";
        $p = new angelPerson();
        $p->shareconn($conn);

        while($n = mysqli_fetch_object($r,"angelFundNote")){
            $date = date_create($n->noteDate);

            $p->id = $n->investorID;
            $u = $p->findMatchingPerson(false);
            $user = $u->firstname . " " . $u->lastname;
            $outputDate = date_format($date, "m/d/Y");
            if ($n->investorID == $investorID) // if the note is owned by this investor
                $editLink = "<a href='javascript:onclick=editFundNote($fund->id, $n->id)'>edit</a> &nbsp &nbsp
                             <a href='javascript:onclick=deleteFundNote($fund->id, $n->id)'>delete</a>";
            else
                $editLink = "";

            if ($n->public)
                $public = " <span style='color:blue'>(public)</span> ";
            else
                $public = "";


            $notes = nl2br(stripcslashes($n->notes));
            //angeldie($notes);
            $page .=    "<div class='companyPageNoteRow'>
                         <div style='display:table-cell; font-size: 14px; '>$notes</div></div>";
            if ($n->type == "valuation update") {
                $acct = "$" . number_format($n->capitalAccount);

                $page .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>Current capital account: $acct</div></div>";
            } else if ($n->type == "exit prediction") {
                $predict = "$" . number_format($n->predictedReturn);
                $page .=    "<div class='companyPageNoteRow'>
                              <div style='display:table-cell; font-size: 12px; '>Predicted Return: $predict</div></div>";
            }
            $page .=     "
                         <div class='companyPageNoteRow'>
                         <div style='display:table-cell; color:#666666'>$outputDate $public $n->type with $user &nbsp &nbsp $editLink</div></div>

                         <div style='display:table-cell;'>&nbsp</div>

                                    ";
        }
        $page .=    "</div>";   // table div

        $page .= "</div>";     // fund page

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // clear



        return($page);


    }

 /*
  * create a left side summary for fund profile and notes pages. if $edit is true, then on notes page and want to show edit link and founders
  */
    function fundLeftSideSummary($fund, $investorID, $edit) {

        /*
         * get some summary info - total committed, invested, returned.
         */
        $inv = new angelFundInvest();
        $conn = $inv->connect();
        $ti = $inv->totalInvestment($fund, $investorID);;
        $totalInvested = "$" . number_format($ti);
        $inv->fundID = $fund->id;
        $inv->investorID = $investorID;
        $inv = $inv->findMatchingCommit();
        $totalCommitted = "$" . number_format($inv->investAmount);

        /*
         * get the total return from the distributions for that fund.
         */
        $po = new angelPayout();
        $po->shareconn($conn);
        $tr = $po->totalFundReturn($fund, $investorID);
        $totalReturned = "$" . number_format($tr);

        $ROI = "$" . number_format($tr - $ti);
        if ($ti == 0)
            $ROIpercent = "N/A";
        else
            $ROIpercent = number_format(100*($tr-$ti)/$ti,0) . "%";

        $page = "<div class='companyPageNavDiv'>";

        $page .= "<span style='font-size:16px; font-weight: bold '> $fund->name </span>";

        $page .= "<p /><span style='font-size:12px; font-weight: lighter '> $fund->description </span>";
        if ($fund->url != "")
            $page .= "<p />&nbsp &nbsp<span style='font-size:12px; font-weight: lighter '> <a href='$fund->url' target='_blank'>$fund->url</a> </span>";

        if ($edit) {
            if (angelHasEditRights($investorID, $fund->creatorID))
                $editCo = "<a href='/fundPage?id=$fund->id&editFund'>edit fund</a>";
            else {
                $editCo = "<a href='javascript:onclick=fundEditRequest(\"$fund->name\")'>Request right to edit</a>";
            }

            $page .= "<p></p>$editCo";
        } else {
            /*
             * show link back to notes page
             */
            $page .= "<p></p><a href=/fundPage?id=$fund->id>Fund Notes</a>";
        }
        $page .= "<p />Fund Status: <span style='font-size:12px; font-weight: lighter '> $fund->status </span>";


        $page .= "<div style='border: 1px solid black; margin: 15px 0px 5px 0px'></div>";
        $page .= "<table>";
        $page .=    "<tr><td>Total Committed: </td><td style='text-align:right'>$totalCommitted</td></tr>";
        $page .=    "<tr><td>Total Invested: </td><td style='text-align:right'>$totalInvested</td></tr>";
        $page .=    "<tr><td>Returned: </td><td style='text-align:right'>$totalReturned</td></tr>";
        if ($tr > 0) {
            $page .=    "<tr><td colspan='2'><hr></td></tr>";
            $page .=    "<tr><td>ROI: </td><td  style='text-align:right'>$ROI</td></tr>";
            $page .=    "<tr><td></td>     <td  style='text-align:right'>$ROIpercent</td></tr>";

        }

        $page .=  "</table>";


        $page .= "</div>";
        return($page);
    }


    /*
     * return new fund note form for input person ($id) and fund ($fundID)
     */
    function newFundNoteForm($id, $fundID) {

        $invForm = "";
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#fundPredictedReturn').number(true);
                $('#fundCapitalAccount').number(true);
            });</script>";

        $invForm .= "<form id='addFundNote' action='/fundPage.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";

        $invForm .= "<input type=hidden value=$fundID name='id'>";

        $invForm .= '<div class="angelinput">';

        $invForm .= "<table width='100%'><thead>
        <tr>
            <th id='fundNoteTypeHeader' valign=top width='10%'>Type of Note<span style='color:red'>*</span></th>
            <th id='fundNoteDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='fundNoteNoteHeader' valign=top width='10%'>Note<span style='color:red'>*</span></th>
         </tr></thead>";

        $selectedType = 'Meeting';         // default note type is a meeting

        /*
         * Type
         */

        $invForm .= '<tr><td valign=top><select onchange="fundNoteTypeSelector()" id="fundNoteType" name="type">';

        foreach ($GLOBALS["companyNoteTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';
        $invForm .= "<td valign=top><input size='11%' type=text id='datepickerFundNote' name='date' value=''></td>";

        $invForm .= "<td valign=top><textarea rows=6 cols=70 id='fundNote' name='fundNote'></textarea></td></tr>";


        $invForm  .= "<tr><td colspan='3'><hr></td></tr>";

        $invForm .= "<tr><td colspan='4'><div id='fundExitPredictionDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Predicted Return</th>
                            </tr>
                            <tr>
                                <td><input type='text' id='fundPredictedReturn' name='fundPredictedReturn' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr>";

        $invForm .= "<tr><td colspan='4'><div id='fundValuationDiv' style='display:none; width:650px'>";
        $invForm .= "    <table style='width:600px'>
                            <tr>
                                <th>Current Capital Account</th>
                            </tr>
                            <tr>
                                <td><input type='text' id='fundCapitalAccount' name='fundCapitalAccount' size='10'></td>
                            </tr>


                        </table>
                    </div></td>
                    </tr></table>";

        $invForm .= "<input type='hidden' id='fundNoteAction' name='action' value='addFundNote'>";
        $invForm .= "<input type='hidden' id='fundNoteUpdate' name='update' value=''>";
        $invForm .= '<p></p> <button type="button" id="newFundNoteButton" onclick="addOrUpdateFundNote()">Add Note</button>
                     &nbsp &nbsp &nbsp;  <input type="checkbox" id="fundPublic" name="public" value="1">  &nbsp;
                     <span id="fundPublicText">Public? If checked, this note will be visible to other Angelcalc users</span>
                    </form></div>';

        return $invForm;

    }  /* newFundNoteForm */

    function fundProfileBody($inputFund, $investorID, $prospect) {

        // angeldie("fundProfileBody: " . serialize($co));

        $name           = htmlentities($inputFund->name, ENT_QUOTES);
        $otherNames     = htmlentities($inputFund->other_names, ENT_QUOTES);
        $fulldesc       = htmlentities($inputFund->description, ENT_QUOTES);
        $email          = htmlentities($inputFund->email, ENT_QUOTES);
        $url            = htmlentities($inputFund->url, ENT_QUOTES);
        $inceptionYear  = htmlentities($inputFund->inceptionYear, ENT_QUOTES);
        $wireBank        = htmlentities($inputFund->wireBank, ENT_QUOTES);
        $wireBankAddress = htmlentities($inputFund->wireBankAddress, ENT_QUOTES);
        $wireBankRouting = htmlentities($inputFund->wireBankRouting, ENT_QUOTES);
        $wireBankSWIFT   = htmlentities($inputFund->wireBankSWIFT, ENT_QUOTES);
        $wireBankAcct    = htmlentities($inputFund->wireBankAcct, ENT_QUOTES);
        $wireFurtherName = htmlentities($inputFund->wireFurtherName, ENT_QUOTES);
        $wireFurtherAcct = htmlentities($inputFund->wireFurtherAcct, ENT_QUOTES);


        $id             = $inputFund->id;

        $fidTOname = array();

        $inputFund->createMap($fidTOname, "fundtable", "id", "name");

        $conn = $inputFund->connect();

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        /*
        * Fund summary on the left
        */
        if ($this->error != "")
            $page .= "<script>$(document).ready( function() {\$.notify('$this->error', 'error');} );</script>";
        elseif ($this->message != "")
            $page .= "<script>$(document).ready( function() {\$.notify('$this->message', 'success');} );</script>";

        $page .= $this->fundLeftSideSummary($inputFund, $investorID, false);




        /*
         * Editable stuff on the right
         */


        $page .= "<div class='companyPageNotesDiv'>";

        $page .= "<form id='updateFund' action='/fundPage?id=$inputFund->id' enctype='multipart/form-data' method='post'>";
        $page .= "<table class=''>";

        /*
         * Fund Name
         */

        $page .= "<td id='fundNameLabel'>Name</td><td valign=top>
            <script>";
        if ($id == 0)
            $page .= "preload_data_cos = 0;";
        else {
            $fundName = $fidTOname[$id];
            $page .= "preload_data_cos = {id:$id, text:'$fundName'};";
        }

        $page .= '
        $(document).ready(function () {
            $("#fundName").select2({
                placeholder: "Select a Fund",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $page .= "data: [";
        foreach (array_keys($fidTOname) as $id) {
            $co = mysqli_escape_string($conn, $fidTOname[$id]);
            if ($needComma)
                $page .= ",";

            $page .= "{id:$id, text:'$co'}";

            $needComma = true;
        }
        $page .= "]";


        $page .= '});
            $("#fundName").select2("data", preload_data_cos);';

        $page .= '});
                      </script>';

        $page .= '<input type="hidden" onchange="fundMustBeNew()" id="fundName" name="fundName" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">Enter or change a name by simply typing it above.</span>
                 </td>';

        $page .= "<tr><td>Inception Year</td>
                       <td><input  style='width:420px' id='inceptionYear' name='inceptionYear' type='text' value='$inceptionYear' /></td></tr>";

        $page .= "<tr><td  id='fundDescriptionLabel'>Description</td>
                       <td><textarea rows=6 cols=70 id='fundDescription' name='fundDescription'>$fulldesc</textarea></td></tr>";

        $page .= "<tr><td>URL</td>
                       <td><input  style='width:420px' id='fundURL' name='fundURL' type='text' value='$url' /></td></tr>";

        $page .= "<tr><td>Other/Former Names</td>
                       <td><input  style='width:420px' id='fundOtherNames' name='fundOtherNames' type='text' value='$otherNames' /></td></tr>";

        $page .= "<tr><td colspan='2'><hr></td></tr>";

        $page .= "<tr><td>Bank Name for Wires</td>
                       <td><input  style='width:420px' id='fundWireBank' name='fundWireBank' type='text' value='$wireBank' /></td></tr>";
        $page .= "<tr><td>Bank Address for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankAddress' name='fundWireBankAddress' type='text' value='$wireBankAddress' /></td></tr>";
        $page .= "<tr><td>Bank Routing for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankRouting' name='fundWireBankRouting' type='text' value='$wireBankRouting' /></td></tr>";
        $page .= "<tr><td>Bank SWIFT for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankSWIFT' name='fundWireBankSWIFT' type='text' value='$wireBankSWIFT' /></td></tr>";
        $page .= "<tr><td>Bank Account for Wires</td>
                       <td><input  style='width:420px' id='fundWireBankAcct' name='fundWireBankAcct' type='text' value='$wireBankAcct' /></td></tr>";
        $page .= "<tr><td>For further credit to name</td>
                       <td><input  style='width:420px' id='fundWireFurtherName' name='fundWireFurtherName' type='text' value='$wireFurtherName' /></td></tr>";
        $page .= "<tr><td>For further credit to acct</td>
                       <td><input  style='width:420px' id='fundWireFurtherAcct' name='fundWireFurtherAcct' type='text' value='$wireFurtherAcct' /></td></tr>";



        $page .= "</table>";


        if ($inputFund->id == 0) {    // new fund
            $page .= '<p></p> <button type="button" id="updateFundButton" onclick="addOrUpdateFund()">Create Fund</button>';
            $page .= "<input type='hidden' id='fundEditAction' name='action' value='createFund'>";
        } elseif (angelHasEditRights($investorID, $inputFund->creatorID)) {
            $page .= '<p></p> <button type="button" id="updateFundButton" onclick="addOrUpdateFund()">Update Fund</button>';
            $page .= "<input type='hidden' id='fundID' name='id' value='$inputFund->id'>";
            $page .= "<input type='hidden' id='savedFundName' name='savedFundName' value='$inputFund->name'>";
            $page .= "<input type='hidden' id='fundEditAction' name='action' value='updateFund'>";
        } else {
            $page .= "<a href='javascript:onclick=fundEditRequest($inputFund->id, \"$inputFund->name\")'>Request ability to edit this fund</a>";
        }

        $page .= "</div>";     // fund page

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // clear



        return($page);


    }   // fundProfileBody()


}