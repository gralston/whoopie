<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/14/15
 * Time: 10:58 AM
 */

class whoopie {

    public $error = "";
    public $view;

    function __construct() {
        $this->view = new whoopie_view();
    }
    /*******************
     * 
     * This method displays the logged in home page for modeling 
     */
    function showWhoopieHomePage() {
        $this->view->error = $this->error;
        $body = $this->view->whoopieHomePageBody();
        $this->view->outputHTML($body);

    }

    function showWhoopieTesterPage() {
        $this->view->error = $this->error;
        $body = $this->view->whoopieTesterPageBody();
        $this->view->outputHTML($body);

    }
   
    /*******************
     * This method displays the help page
     */
    function showHelpPage($investor, $whatHelp) {


        $this->view->error = $this->error;
        $body = $this->view->HelpPageBody($investor, $whatHelp);
        $this->view->footer = "";           // only because I can't get it to work just now!!! Shows up in the middel of the page
        $this->view->outputHTML($body);
    }

    /*
     * called from ajaxNextWhoopieEvent to see if there are any new events
     */
    function checkForEvents($gameID, $playerID, $lastEventID, &$events) {
        /*
         * first get the game from the games table
         *   if in waiting state, find out how many have joined. for now, if 2, start the game by 
         *   returning a gameOn event, with the deck from player 0
         */

        // return all events newer than the lastEventID
        //
        $gameLog = new whoopieGameLog();
        $gameLog->connect();
        $rows = $gameLog->getRows("gamelog", "gameid = $gameID and id > $lastEventID" );
        if ($rows == null or $rows == -1) {
            error_log("checkForEvents - bad return from gamelog");
            return(0);
        }
        $count = 0;
        while($l = mysqli_fetch_object($rows,"whoopieGameLog")) {
            $deck = unserialize($l->deck);
            if ($l->name == "gameReset") {
                $count = 0; // skip everything beforehand
                $events = array();
            } else {
                $events[$count] = array("type"=>$l->name,  "gameID"=>$gameID, "playerID"=>$l->playerid, 
                                    "playerName"=>$l->playername, "lastEventID"=>$l->id, "cardPlayed"=>$l->cardplayed, 
                                    "bidMade"=>$l->bidmade, "deck"=>$deck );
                $count++;
            }
        }
        return($count);

    }

    function ajaxNextWhoopieEvent() {

        $gameID = $_GET["gameID"];
        $playerID = $_GET["playerID"];
        $lastEventID = $_GET["lastEventID"];
        $events = array();

        $json = "{";
        $json .= '"' . "events" .  '": ';
        $count = $this->checkForEvents($gameID, $playerID,  $lastEventID, $events);
        error_log("func  ajaxNextWhoopieEvent  lasteventID, count: " . angelParens($lastEventID, $count));
        $json .= json_encode($events);

        $json .= "}";

        
        error_log("func  ajaxNextWhoopieEvent  json: " . $json);

        echo($json);
    }

    
    function ajaxWhoopieRequest($request) {

        error_log("func ajaxWhoopieRequest (request, card) playerID" . angelParens($request->whoopieRequest->name, $request->whoopieRequest->card), $request->whoopieRequest->playerID);
        // error_log("func ajaxWhoopieRequest player name:" . $request->whoopieRequest->playerName);
        $gameLog = new whoopieGameLog();
        $conn = $gameLog->connect();
        $debug = false;

        $playerName = $request->whoopieRequest->playerName;

        $whoopieResponse = array("name"=>"joinedGame", "gameID"=>"0", "playerID"=>"-1", "lastID"=>"0");

        $json = "{";
        $json .= '"' . "whoopieResponse" .  '": ';


        if ($request->whoopieRequest->name == "joinGame") {
            if (strtolower($playerName) == "geoff")
                $whoopieResponse["playerID"] = "0";
            elseif (strtolower($playerName) == "jon")
                $whoopieResponse["playerID"] = "1";
            elseif (strtolower($playerName) == "steven")
                $whoopieResponse["playerID"] = "2";
            elseif (strtolower($playerName) == "elizabeth")
                $whoopieResponse["playerID"] = "3";

            $gameLog->name = "playerJoined";
            $gameLog->playerid = $whoopieResponse["playerID"];
            $gameLog->playername = $playerName;
            $gameLog->gameid = 1;
            $gameLog->bidmade = 0;
            $gameLog->cardplayed = "";

            $gameLog->deck = serialize($request->whoopieRequest->deck);
            error_log("func ajaxWhoopieRequest joingame: deck:" . $gameLog->deck);


            

            // now update game entry and see if game is live
            $game = new whoopieGame();
            $gameLog->shareconn($conn);
            $game->id = 1;  // we gotta do some work if we want to support multiple games!
            $game = $game->getEntry();
            
            if (strpos($game->playernames, $playerName) == false) {
                    // first time for this player!
                    
                $whoopieResponse["lastID"] = $gameLog->insertRow();
                $game->players++;
                if ($game->players == $game->playercount or $debug) {
                    error_log("func ajaxWhoopieRequest gameOn");
                    $game->status = "gameOn";          
                    // write gameOn event to gameLog
                    $gameLog->name = "gameOn";
                    $gameLog->insertRow();
                }
                $game->playernames .= ":" . $playerName;
                
                $game->updateThisRow();     // update active player count (and maybe status)
            } else {
                $whoopieResponse["name"] = "Error: " . $playerName . " is already playing. Please try again.";
            }
            
        } else if ($request->whoopieRequest->name == "dealForFirstDeal") {

            $whoopieResponse["name"] = "receivedFirstDeal";
            $gameLog->name = "dealForFirstDeal";
            $gameLog->playerid = $request->whoopieRequest->playerID;
            $gameLog->playername = $request->whoopieRequest->playerName;
            $gameLog->gameid = 1;
            $gameLog->bidmade = 0;
            $gameLog->cardplayed = "";

            
            $gameLog->deck = serialize($request->whoopieRequest->deck);
            $whoopieResponse["lastID"] = $gameLog->insertRow();
        } else if ($request->whoopieRequest->name == "newDeal") {
            $whoopieResponse["name"] = "receivedDeal";
            $gameLog->name = "newDeal";
            $gameLog->playerid = $request->whoopieRequest->playerID;
            $gameLog->playername = $request->whoopieRequest->playerName;
            $gameLog->gameid = $request->whoopieRequest->gameID;;
            $gameLog->bidmade = 0;
            $gameLog->cardplayed = "";

            
            $gameLog->deck = serialize($request->whoopieRequest->deck);

            $whoopieResponse["lastID"] = $gameLog->insertRow();
        } else if ($request->whoopieRequest->name == "newBid") {
            $whoopieResponse["name"] = "receivedBid";
            $gameLog->name = "newBid";
            $gameLog->playerid = $request->whoopieRequest->playerID;
            $gameLog->playername = $request->whoopieRequest->playerName;
            $gameLog->gameid = $request->whoopieRequest->gameID;
            $gameLog->bidmade = $request->whoopieRequest->bid;
            $gameLog->cardplayed = "";

            $whoopieResponse["lastID"] = $gameLog->insertRow();
        } else if ($request->whoopieRequest->name == "cardPlayed") {
            $whoopieResponse["name"] = "receivedCard";
            $gameLog->name = "cardPlayed";
            $gameLog->playerid = $request->whoopieRequest->playerID;
            $gameLog->playername = $request->whoopieRequest->playerName;
            $gameLog->gameid = $request->whoopieRequest->gameID;
            $gameLog->bidmade = 0;
            $gameLog->cardplayed = $request->whoopieRequest->card;

            $whoopieResponse["lastID"] = $gameLog->insertRow();
        } else if ($request->whoopieRequest->name == "resetGame") {

            // now update game entry and see if game is live
            $whoopieResponse["name"] = "gameReset";
            $game = new whoopieGame();
            $gameLog->shareconn($conn);
            $game->id = 1;  // we gotta do some work if we want to support multiple games!
            $game = $game->getEntry();
            $game->players = 0;
            $game->playernames = "";
            $game->status = "wait";          
            // write gameOver event to gameLog
            $gameLog->name = "gameReset";
            $gameLog->playerid = 0;
            $gameLog->playername = $request->whoopieRequest->playerName;
            $gameLog->gameid = 1;
            $gameLog->bidmade = 0;
            $gameLog->cardplayed = "";
            $game->firstlogid = $gameLog->insertRow(); //   first id for next game is entry AFTER the gameOver entry

        
            $game->updateThisRow();     // update active player count (and maybe status)
            
        } else {
            // unknown request
            $whoopieResponse["name"] = "unknownRequest";
            error_log("func ajaxWhoopieRequest unkownRequest: " . $request->whoopieRequest->name);
        }

        $json .= json_encode($whoopieResponse);

        $json .= ",";
        $json .= '"' . "deck" .  '": ';
        $json .= json_encode($request->whoopieRequest->deck);
        $json .= "}";

        error_log("func ajaxWhoopieRequest json: " . $json);

        echo($json);

    }

    function ajaxWhoopieNext($gameID, $playerNO) {

        $json = "{";

        $sql = "SELECT p.id, COUNT(m.id) as count  FROM persontable  p LEFT JOIN modeltable m ON p.id = m.investor_id GROUP BY p.id";
        $r = $p->getRowsCustom($sql);    // if we don't order these, they come out in pseudo random order


        $rows = array();
        while($row = mysqli_fetch_row($r)) {
            $rows[] = $row;
        }
        $json .= '"' . "models" .  '": ';
        $json .= json_encode($rows);
        $json .= ",";

        $sql = "SELECT p.id, COUNT(a.id) as count  FROM persontable  p LEFT JOIN angelinvests a ON p.id = a.investor_id GROUP BY p.id";
        $r = $p->getRowsCustom($sql);    // if we don't order these, they come out in pseudo random order

        $rows = array();
        while($row = mysqli_fetch_row($r)) {
            $rows[] = $row;
        }
        $json .= '"' . "investments" .  '": ';
        $json .= json_encode($rows);
        $json .= "}";

        //throw new Exception($json);     // debug
        echo($json);
    }


    /*****************************************************************************
     * @param $investorID
     *
     * Called from browser to send feedback. Format an email and send it to geoff
     */
    function ajaxFeedback($investorID) {

        if (array_key_exists("email",$_POST))
            $email = $_POST['email'];
        else
            $email ="";

        if ($investorID == 0)
            $email .= "[guest account - no email provided]";
        else {
            $p = new angelPerson();
            $p->connect();
            $p->id = $investorID;
            $p = $p->findMatchingPerson(false);
            if ($p != NULL)
                $email .= "[" . $p->email . "]";
            else
                $email = "[check into this: could not find email for id: " . $investorID . "]";
        }


        $body = "Feedback from: " .  $_POST['name'] . " / " . $email;
        $body .= "
--------------------------------------------------------------------------------
";
        $body .= $_POST['message'];
        $body .= "
--------------------------------------------------------------------------------
";

        $body .= serialize($_POST);
        $body .= "
--------------------------------------------------------------------------------
";
        if ($_POST['radio_list_value'] == 1)
            $answer = "Yes";
        elseif ($_POST['radio_list_value'] == 2)
            $answer = "No";
        else
            $answer = "no answer";

        $body .= "Is Angelcalc useful? " . $answer;



        angelSendEmail("Angelcalc Feedback", $body, "Geoff", "geoff@yahoo.com");

    }

    /********************************************************************************
     * @param $investorID   -- session id
     * @param $modelID      -- model to be shared
     *
     * Turn on sharing for the input model
     */

    function ajaxShareModel($modelID) {

        // angellog("ajax model share request. ID: " . $modelID);    // logged by model.php

        $mod = new angelModel();
        $mod->id = $modelID;
        $conn = $mod->connect();
        $modelInput = $mod->getEntry();    // get entry of the requested model
        if ($modelInput == 0) {
            echo("error='No Such Model ID'");
            die();
        }

        if ($modelInput->investor_id != $_SESSION["loggedInUserID"]) {
            angellog("Request to share unowned model (model, loggedInUser): " .  "(" . $modelID . ',' . $_SESSION["loggedInUserID"] . ")");
            echo("error='You do not own this model");
            die();
        }
        $hash = md5( rand(0,10000) );             // Generate random 32 character hash

        $r = $mod->updateRow("sharing='$hash'", "id=$modelID");
        if ($r == 0)
            echo("error: " . $mod->error() );
        else
            echo("hash=" . $hash);
    }

    /********************************************************************************
     * @param $investorID   -- session id
     * @param $modelID      -- model to be shared
     */

    function ajaxUnShareModel($modelID) {

        // angellog("ajax model unshare request. ID: " . $modelID);   // logged by model.php

        $mod = new angelModel();
        $mod->id = $modelID;
        $conn = $mod->connect();
        $modelInput = $mod->getEntry();    // get entry of the requested model
        if ($modelInput == 0) {
            echo("status='No Such Model ID'");
            die();
        }

        if ($modelInput->investor_id != $_SESSION["loggedInUserID"]) {
            angellog("Request to unshare unowned model (model, loggedInUser): " .  "(" . $modelID . ',' . $_SESSION["loggedInUserID"] . ")");
            echo("status='You do not own this model");
            die();
        }

        $r = $mod->updateRow("sharing=''", "id=$modelID");
        if ($r == 0)
            echo("status=" . $mod->error() );
        else
            echo("status=ok&");
    }

    /**********************************************************
     *    This method returns a json with two entries. the first is a table of ids and model counts
     *    and the second is a table of ids and investment counts
     *
     * @param $modelID
     * @param $hash  -- if this is an actual hash this is a shared request. Be gentle on who can see it.
     */

    function ajaxLoadModelandInvestCounts() {

        $p = new angelPerson();
        $p->connect();
        $json = "{";

        $sql = "SELECT p.id, COUNT(m.id) as count  FROM persontable  p LEFT JOIN modeltable m ON p.id = m.investor_id GROUP BY p.id";
        $r = $p->getRowsCustom($sql);    // if we don't order these, they come out in pseudo random order


        $rows = array();
        while($row = mysqli_fetch_row($r)) {
            $rows[] = $row;
        }
        $json .= '"' . "models" .  '": ';
        $json .= json_encode($rows);
        $json .= ",";

        $sql = "SELECT p.id, COUNT(a.id) as count  FROM persontable  p LEFT JOIN angelinvests a ON p.id = a.investor_id GROUP BY p.id";
        $r = $p->getRowsCustom($sql);    // if we don't order these, they come out in pseudo random order

        $rows = array();
        while($row = mysqli_fetch_row($r)) {
            $rows[] = $row;
        }
        $json .= '"' . "investments" .  '": ';
        $json .= json_encode($rows);
        $json .= "}";

        //throw new Exception($json);     // debug
        echo($json);
    }


    /**********************************************************
     *    This method returns a model via a query string to the browser.
     *
     * @param $modelID
     * @param $hash  -- if this is an actual hash this is a shared request. Be gentle on who can see it.
     */

    function ajaxModel($modelID, $hash) {

        // angellog("ajax load model request. (ID,hash): " . "(" . $modelID . ',' . $hash . ")");   // logged by model.php

        $mod = new angelModel();
        $mod->id = $modelID;
        $conn = $mod->connect();

        $modelInput = $mod->getEntry();    // get entry of the requested model
        if ($modelInput == null) {
            echo("error='No Such Model ID'");
            die();
        }

        if ($hash != "") {
            if ($modelInput->sharing != $hash) {
                angellog("Request for unshared model: " . $modelID);
                echo("error=This model is no longer shared");
                die();
            }
        } else {    // if not a legit shared model request, then only the owner gets to load their model

            if ($modelInput->investor_id != $_SESSION["loggedInUserID"]) {
                angellog("Request to load unowned model (model, loggedInUser): " .  "(" . $modelID . ',' . $_SESSION["loggedInUserID"] . ")");
                echo("error='You do not own this model! (model,user):" . "(" . $modelInput->investor_id . "," .  $_SESSION["loggedInUserID"] . ")");      // debug
                //echo("error='You do not own this model!");
                die();
            }
        }
        $modelInput->shareConn($conn);
        $modelInput->incrementRefcount();

        $queryString = http_build_query($modelInput);

        $conv = new angelConvert();
        $conv->shareconn($conn);
        $rows = $conv->getRows('converttable', "model_id=$mod->id ORDER BY id");    // if we don't order these, they come out in pseudo random order
        $i = 0;
        while($conv = mysqli_fetch_object($rows,"angelConvert")){
            /*
             *  create query string for each convertible to be parsed by jscript on the browser
             */
            $queryString .= "&CONV".$i."convType=" . $conv->type .
                            "&CONV".$i."convAmount=" . $conv->yourInvestment .
                            "&CONV".$i."convTotal=" . $conv->totalInvestment .
                            "&CONV".$i."discount=" . $conv->discount .
                            "&CONV".$i."cap=" . $conv->cap .
                            "&CONV".$i."custom=" . $conv->custom .
                            "&CONV".$i."preexist=" . $conv->preexist .
                            "&CONV".$i."shares=" . $conv->shares;
            $i++;
        }

        //angeldie($queryString);

        echo($queryString);

    }
    /*******************
     * This method returns all data angelcalc needs for the input investor via json
     */
    function ajaxAngelcalcData($investorID) {

        angellog("ajax AngelcalcData request. ID: " . $investorID);   // logged by model.php

        $db = new angeldb();
        $conn = $db->connect();

        $json = "{";

        /*
         * Get angel investment data and package up as a json
         */
        $json .= '"' . "investments" .  '": ';
        $rows = $this->getInvestments($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "companies" .  '": ';
        $rows = $this->getCompanies($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $rows = $this->getTracker($investorID, $conn);
        $json .= '"' . "tracker" .  '": ';
        $json .= json_encode($rows);
        $json .= ",";

        $rows = $this->getTrackerPeople($investorID, $conn);
        $json .= '"' . "trackerpeople" .  '": ';
        $json .= json_encode($rows);
        $json .= ",";

        $rows = $this->getTrackerCompanies($investorID, $conn);
        $json .= '"' . "trackercompanies" .  '": ';
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "payouts" .  '": ';
        $rows = $this->getPayouts($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "valuationnotes" .  '": ';
        $rows = $this->getValuationNotes($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";
        // $rows = $this->getCompanyFounders($investorID, $conn);

        /*
         * Get fund investment data and add to the package
         */
        $json .= '"' . "fundinvestments" .  '": ';
        $rows = $this->getFundInvestments($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "funds" .  '": ';
        $rows = $this->getFunds($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "fundpayouts" .  '": ';
        $rows = $this->getFundPayouts($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "fundvaluationnotes" .  '": ';
        $rows = $this->getFundValuationNotes($investorID, $conn);
        $json .= json_encode($rows);
        $json .= ",";

        $json .= '"' . "fundmodels" .  '": ';
        $rows = $this->getFundModels($investorID, $conn);
        $json .= json_encode($rows);
        // $json .= ",";

        $json .= "}";

        echo($json);

    }

    function getInvestments($investorID, $conn) {
        $ai = new angelInvest();
        $ai->shareconn($conn);
        $r = $ai->getRows("angelinvests", "investor_id=$investorID ORDER BY inv_date DESC");

        if ($r == NULL) {
            echo("AC-ERR='getrows (investments) failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }

    function getCompanies($investorID, $conn) {
        $rows = array();

        angelGetMyCompanies($rows, $investorID);

        return ($rows);

    }

    /*
     * tracker methods:
     * Return (1) all rows from tracker table (2) company names and ids that are tracked (3) people names and ids
     * that are tracked.
     */
    function getTracker($investorID, $conn) {
        $tr = new angelTracker();
        $tr->shareconn($conn);
        $r = $tr->getRows("trackertable", "investorID=$investorID ORDER BY last_update DESC");

        if ($r == NULL) {
            echo("AC-ERR='getrows (tracker) failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $row = $this->updateTracker($row);
            $rows[] = $row;
        }

        return ($rows);

    }
    /*
     * this method updates the status and next contact date of the tracker specified by the input
     * associative array row if necessary. A status update always means setting it to live.
     *
     * Deciding whether
     * a tracker is live or not isn't so simple. Here's the algorithm:
     *
     *    Only do this if tracker status is not a live one (live or live:contacted)
     *
     *    if lastStatusUpdate is null (never been live)
     *      if start=now and current date < 1 frequency from trackerDate   then should be live
     *      if start=next m and current date is 1 month from trackerDate then s.b. live
     *      if start=next q and current date is 1 quarter from trackerDate then s.b. live
     *
     *      now update next tracker date based on frequency
     *
     *    otherwise
     *    if (current date > next contact date) set to live and update next contact date based on freq.
     *
     *    if (current date > trackerDate)
     *              if frequency = month and current year > tracker year or current month != tracker month
     *                          s.b. live
     *              if frequency = quarter and current year > tracker year or current quarter != tracker quarter
     *                          s.b. live
     *              if frequency = year and current year > tracker year and current month >= tracker month
     *                          s.b. live
     */
    function updateTracker($row) {
        $freq = $row["frequency"];
        $start = $row["start"];
        $status = $row["status"];
        $currYear = (int)date("Y");
        $currMonth = (int)date("m");
        $currQuarter = $this->returnQuarter($currMonth);

        $today = date('Y-m-d');
        $nextContact = $row["nextContactDate"];

        $todayD = strtotime("today");
        if ($nextContact == null or $nextContact == "")
            $nextContactD = 0;
        else
            $nextContactD = strtotime(str_replace('-', '/', $nextContact));


        // error_log("updateTracker: id | (today, next)" . $row["id"] . " | " . angelParens($todayD, $nextContactD));

        if ($status == "" or substr($status, 0, 4) != "live") {        // tracker is not already live
            if ($todayD > $nextContactD) {
                $row["status"] = "live";
                $row["nextContactDate"] = $this->updateTrackerContactDate($row);
            }

        }
        return($row);

    /*
     * none of this is used anymore. delete when you are sure it won't come back!
     */
        $trackDate = $row["trackerDate"];                   // date entered
        if ($trackDate != null and $trackDate != "") {
            $trackYear = (int)substr($trackDate, 0,4);
            $trackMonth = (int)substr($trackDate, 5,2);
            $trackQuarter = $this->returnQuarter($trackMonth);
        } else {
            $trackYear = $currYear;
            $trackMonth = $currMonth;
            $trackQuarter = $currQuarter;
        }

        $lastDate = $row["lastStatusUpdate"];               // date of last status update
        if ($lastDate != null and $lastDate != "") {
            $lastYear = (int)substr($lastDate, 0,4);
            $lastMonth = (int)substr($lastDate, 5,2);
            $lastQuarter = $this->returnQuarter($lastMonth);
        }




        /*
         *  Handle initial entry cases
         */
        if ($lastDate == null or $lastDate == "" or $lastDate == $trackDate) {
            if ($start == "Now") {
                $nextContactD = 0;  // forces this to go live
            } elseif ($start == "Next Month")  {
                if ($currYear == $trackYear) {
                    error_log("here: " . angelParens($currMonth, $trackMonth));
                    if ($currMonth == $trackMonth + 1) {
                        $this->makeTrackerLive($row);
                        return("live");
                    }
                } elseif ($currYear == $trackYear + 1 and $currMonth == 1 and $trackMonth == 12) {
                    $this->makeTrackerLive($row);
                    return("live");
                }
            } elseif ($start == "Next Quarter")  {
                if ($currYear == $trackYear) {
                    if ($currQuarter == $trackQuarter + 1) {
                        $this->makeTrackerLive($row);
                        return("live");
                    }
                } elseif ($currYear == $trackYear + 1 and $currQuarter == 1 and $trackQuarter == 4) {
                    $this->makeTrackerLive($row);
                    return("live");
                }
            }
        } else {
        /*
         * Now handle frequency cases
         */
            if ($freq == "monthly") {
                if ($currYear > $trackYear or $currMonth > $trackMonth) {
                    $this->makeTrackerLive($row);
                    return("live");
                }
            } elseif ($freq == "quarterly") {
                if ($currYear > $trackYear or $currQuarter > $trackQuarter) {
                    $this->makeTrackerLive($row);
                    return("live");
                }
            } elseif ($freq == "yearly") {
                if ($currYear > $trackYear and $currMonth >= $trackMonth) {
                    $this->makeTrackerLive($row);
                    return("live");
                }
            }
        }
        return($status);        // default is to keep current status
    }

    function returnQuarter($month) {
        if ($month >= 1 and $month <= 3)
            return(1);
        elseif ($month > 3 and $month <= 6)
            return(2);
        elseif ($month > 6 and $month <= 9)
            return(3);
        elseif ($month > 9 and $month <= 12)
            return(4);

    }
    /*
     * I believe this is no longer used. delete when sure!
     */
    function makeTrackerLive($row) {
        $tr = new angelTracker();
        $tr->id = $row["id"];
        $tr->type = $row["type"];
        $tr->personID = $row["personID"];
        $tr->companyID = $row["companyID"];
        $tr->investorID = $row["investorID"];
        $tr->trackerDate = $row["trackerDate"];
        $tr->lastStatusUpdate = date('Y-m-d');
        $tr->status = "live";
        $tr->frequency = $row["frequency"];
        $tr->start = $row["start"];
        $tr->notes = $row["notes"];

        $tr->updateThisRow();
    }
    /*
     * changes a tracker to live and sets a new contact date
     */
    function updateTrackerContactDate($row) {

        $tr = new angelTracker();
        $freq = $row["frequency"];

        /*
        * Set next date based on frequency
        */
        if ($freq == "monthly") {
                $nextContactDate = date('Y-m-d', strtotime("+1 month"));

        } elseif ($freq == "quarterly") {
            $nextContactDate = date('Y-m-d', strtotime("+3 months"));

        } elseif ($freq == "yearly") {
            $nextContactDate = date('Y-m-d', strtotime("+1 year"));
        } else
            $nextContactDate = $nextContactDate = date('Y-m-d', strtotime("+1 month")); // should never happen

        $tr->nextContactDate = $nextContactDate;
        $tr->id = $row["id"];
        $tr->type = $row["type"];
        $tr->personID = $row["personID"];
        $tr->companyID = $row["companyID"];
        $tr->investorID = $row["investorID"];
        $tr->trackerDate = $row["trackerDate"];
        $tr->lastStatusUpdate = date('Y-m-d');
        $tr->status = "live";
        $tr->frequency = $row["frequency"];
        $tr->start = $row["start"];
        $tr->notes = $row["notes"];

        $tr->updateThisRow();

        return($nextContactDate);
    }

    function getTrackerPeople($investorID, $conn) {

        $tr = new angelTracker();
        $tr->shareconn($conn);

        $q = "SELECT DISTINCT i.personID,CONCAT(f.firstname, ' ', f.lastname) AS name FROM persontable as f LEFT JOIN trackertable AS i ON f.id = i.personID WHERE i.investorID=$investorID ORDER BY f.lastname ASC";
        $r = $tr->getRowsCustom($q);

        if ($r == NULL) {
            echo("AC-ERR='getrows (trackerpeople) failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }

    function getTrackerCompanies($investorID, $conn) {
        $tr = new angelTracker();
        $tr->shareconn($conn);
        $q = "SELECT DISTINCT i.companyID,f.name FROM companytable as f LEFT JOIN trackertable AS i ON f.id = i.companyID WHERE i.investorID=$investorID ORDER BY f.name ASC";
        $r = $tr->getRowsCustom($q);

        if ($r == NULL) {
            echo("AC-ERR='getrows (trackercompanies) failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }

    function getPayouts($investorID, $conn) {

        $ap = new angelPayout();
        $ap->shareconn($conn);
        $r = $ap->getRows("payouttable", "investorID=$investorID ORDER BY payDate DESC");

        if ($r == NULL) {
            echo("AC-ERR='getrows (distributions) failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }

    function getValuationNotes($investorID, $conn) {

        $companyVals = array();
        $companyExits = array();
        $companySplits = array();

        $no = new angelNote();
        $no->shareconn($conn);
        $r = $no->getRows("notestable", "investorID=$investorID and (type='valuation update' or type='exit prediction' or type='stock split') ORDER BY noteDate DESC, last_update DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing notes failed'");
            die();
        }

        $rows = array();
        /*
         * go through each row, only returning most recent valuation update (the first) and exit prediction
         * and every stock split!
         */
        while($row = mysqli_fetch_assoc($r)) {
            $companyID = $row["companyID"];
            $type = $row["type"];
            if ($type == "valuation update" and !array_key_exists($companyID, $companyVals)) {
                $rows[] = $row;
                $companyVals[$companyID] = $row;
            } elseif ($type == "exit prediction" and !array_key_exists($companyID, $companyExits)) {
                $rows[] = $row;
                $companyExits[$companyID] = $row;
            } elseif ($type == "stock split") {
                $rows[] = $row;
                $companySplits[$companyID] = $row;  // don't actually use this because we take them all
            }
        }

        $rows = array_reverse($rows);               // make sure rows are processed in the correct order
                                                    // which means in this case that splits happen chronologically
        return ($rows);

    }

    function getFundInvestments($investorID, $conn) {
        $fi = new angelfundinvest();
        $fi->shareconn($conn);
        $r = $fi->getRows("fundinvests", "investorID=$investorID ORDER BY invDate DESC");

        if ($r == NULL) {
            echo("AC-ERR='getrows failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }
    function getFunds($investorID, $conn) {

        $rows = array();

        angelGetMyFunds($rows, $investorID);

        return ($rows);

    }

    function getFundPayouts($investorID, $conn) {
        $ap = new angelPayout();
        $ap->shareconn($conn);
        $r = $ap->getRows("fundpayouttable", "investorID=$investorID ORDER BY payDate DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing fund distributions failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }
    function getFundModels($investorID, $conn) {
        $mod = new angelFundCFModel();
        $mod->shareconn($conn);
        $r = $mod->getRows("fundmodeltable", "investorID=$investorID");

        if ($r == NULL) {
            echo("AC-ERR='accessing fund models failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        return ($rows);

    }
    function getFundValuationNotes($investorID, $conn) {

        $fundVals = array();
        $fundExits = array();

        $no = new angelFundNote();
        $no->shareconn($conn);
        $r = $no->getRows("fundnotestable", "investorID=$investorID and (type='valuation update' or type='exit prediction') ORDER BY noteDate DESC, last_update DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing notes failed'");
            die();
        }

        $rows = array();
        /*
         * go through each row, only returning most recent valuation update (the first) and exit prediction
         */
        while($row = mysqli_fetch_assoc($r)) {
            $fundID = $row["fundID"];
            $type = $row["type"];
            if ($type == "valuation update" and !array_key_exists($fundID, $fundVals)) {
                $rows[] = $row;
                $fundVals[$fundID] = $row;
            } elseif ($type == "exit prediction" and !array_key_exists($fundID, $fundExits)) {
                $rows[] = $row;
                $fundExits[$fundID] = $row;
            }
        }

        return ($rows);

    }


    /*******************
     * This method returns the investments for the input investor via json
     */
    function ajaxInvestments($investorID) {

        // angellog("ajax investments request. ID: " . $investorID);   // logged by model.php

        $ai = new angelInvest();
        $ai->connect();
        $r = $ai->getRows("angelinvests", "investor_id=$investorID ORDER BY inv_date DESC");

        if ($r == NULL) {
            echo("AC-ERR='getrows failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        echo json_encode($rows);

    }

    /********************************************************************
     * This method sets the exit status of the input investment then sets
     * the status of every other investment in that company and the company
     * too, then returns a status of the changes.
     */
    function ajaxExit($investorID) {
        $inv = new angelInvest();
        $conn = $inv->connect();
        $id = $_GET["id"];
        $exitType = $_GET["exit"];
        $date = $_GET["date"];
        if ($date != '')
            $date = date('Y-m-d', strtotime(str_replace('-', '/', $date)));
        $acquirer = $_GET["acquirer"];
        $price = $_GET["price"];
        if (! is_numeric($price))
            $price = 0;

        $r = $inv->getRows("angelinvests", "id=$id");
        if ($r == NULL) {
            angellog("ajaxExit: lookup investment failed: " .  $id);
            echo("AC-ERR='lookup investment failed'");
            die();
        }
        $inv = mysqli_fetch_object($r,"angelInvest");

        $co = new angelCompany();
        $co->shareconn($conn);

        $co->id = $inv->company_id;
        $co = $co->findMatchingCompany();
        if ($co == NULL) {
            angellog("ajaxExit: find company failed: company id: " .  $inv->company_id);
            echo("AC-ERR='find company failed'");
            die();
        }


        /*
         * first, update the company
         */
        $co->status = $exitType;
        $co->exitDate = $date;

        $colsAndVals = "status='" . $co->status . "',exitDate='" . $date . "',acquirer='" . $acquirer . "',acquiPrice=" . $price;
        $r = $co->updateRow($colsAndVals, "id=" . $co->id);
        if ($r == NULL) {
            angellog("ajaxExit: company update failed: company name/id" . $co->name . "/" . $co->id);
            echo("AC-ERR='company update failed'");
            die();
        }
        /*
         * then update all the investments in the company
         */
        $r = $inv->getRows("angelinvests", "company_id=" . $co->id);

        while($i = mysqli_fetch_object($r,"angelInvest")) {
            $i->shareconn($conn);
            $i->updateRow("status='" . $co->status . "'", "id=" . $i->id);
        }

        echo("status=ok&");

    }       // ajaxExit

    /********************************************************************
     * This method sets the exit status of the input investment then sets
     * the status of every other investment in that company and the company
     * too, then returns a status of the changes.
     */
    function ajaxConvert($investorID, $investID) {
        $inv = new angelInvest();
        $conn = $inv->connect();
        $inv->id = $investID;
        $inv = $inv->getEntry();

        $date = $_GET["date"];

        if ($date != '')
            $date = date('Y-m-d', strtotime(str_replace('-', '/', $date)));

        $prefShares = $_GET["prefShares"];
        $commonShares = $_GET["commonShares"];
        $prefSeries = $_GET["prefSeries"];
        $modelID = $_GET["convertModel"];
        $interest = $_GET["interest"];

        if (! is_numeric($prefShares))
            $prefShares = 0;
        if (! is_numeric($commonShares))
            $commonShares = 0;
        if (! is_numeric($interest))
            $interest = 0;

        /*
         * the series is either an id referring to a round/series name or a new one.
         */
        $prefSeries = angelRound($prefSeries, $conn);

        /*
         *  update the investment
         */

        $colsAndVals = "converted=1,prefShares=" . $prefShares . ",commonShares=" . $commonShares . ",prefSharesSeries='" . $prefSeries . "',modelID="
                                    . $modelID .  ",conv_date='" . $date . "',invest_amount=invest_amount + " . $interest .
                                    ",shares=" . $prefShares . "+" . $commonShares;
        $r = $inv->updateRow($colsAndVals, "id=" . $inv->id);
        if ($r == NULL) {
            angellog("ajaxConvert: investment update failed: id" . $inv->id);
            echo("AC-ERR='investment update failed'");
            die();
        }

        echo("status=ok&");

    }       // ajaxConvert



    /*******************
     * This method returns the payouts for the input investor via json
     */
    function ajaxPayouts($investorID) {

        // angellog("ajax payouts request. ID: " . $investorID);

        $ap = new angelPayout();
        $ap->connect();
        $r = $ap->getRows("payouttable", "investorID=$investorID ORDER BY payDate DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing distributions failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        echo json_encode($rows);

    }

    /*******************
     * This method returns the valuation and exit notes for the input investor via json. Only the most recent note per company
     * is returned.
     */
    function ajaxValuationNotes($investorID) {

        // angellog("ajax valuation notes request. ID: " . $investorID);
        $companyVals = array();
        $companyExits = array();

        $no = new angelNote();
        $no->connect();
        $r = $no->getRows("notestable", "investorID=$investorID and (type='valuation update' or type='exit prediction') ORDER BY noteDate DESC, last_update DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing notes failed'");
            die();
        }

        $rows = array();
        /*
         * go through each row, only returning most recent valuation update (the first) and exit prediction
         */
        while($row = mysqli_fetch_assoc($r)) {
            $companyID = $row["companyID"];
            $type = $row["type"];
            if ($type == "valuation update" and !array_key_exists($companyID, $companyVals)) {
                $rows[] = $row;
                $companyVals[$companyID] = $row;
            } elseif ($type == "exit prediction" and !array_key_exists($companyID, $companyExits)) {
                $rows[] = $row;
                $companyExits[$companyID] = $row;
            }
        }

        echo json_encode($rows);

    }

    /*******************
     * This method returns a funds database via json for the input investorID
     */
    function ajaxFunds($investorID) {
        $rows = array();

        angelGetMyFunds($rows, $investorID);

        echo json_encode($rows);

    }

    /*******************
     * This method returns the fund investments for the input investor via json
     */
    function ajaxFundInvestments($investorID) {

        // angellog("ajax investments request. ID: " . $investorID);   // logged by model.php

        $fi = new angelfundinvest();
        $fi->connect();
        $r = $fi->getRows("fundinvests", "investorID=$investorID ORDER BY invDate DESC");

        if ($r == NULL) {
            echo("AC-ERR='getrows failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        echo json_encode($rows);

    }
    /*******************
     * This method returns the payouts for the input investor via json
     */
    function ajaxFundPayouts($investorID) {

        // angellog("ajax payouts request. ID: " . $investorID);

        $ap = new angelPayout();
        $ap->connect();
        $r = $ap->getRows("fundpayouttable", "investorID=$investorID ORDER BY payDate DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing fund distributions failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        echo json_encode($rows);

    }
    /*******************
     * This method returns the fund models for the input investor via json
     */
    function ajaxFundModels($investorID) {

        // angellog("ajax payouts request. ID: " . $investorID);

        $mod = new angelFundCFModel();
        $mod->connect();
        $r = $mod->getRows("fundmodeltable", "investorID=$investorID");

        if ($r == NULL) {
            echo("AC-ERR='accessing fund models failed'");
            die();
        }

        $rows = array();
        while($row = mysqli_fetch_assoc($r)) {
            $rows[] = $row;
        }

        echo json_encode($rows);

    }

    /*******************
     * This method returns the fund valuation and exit notes for the input investor via json. Only the most recent note per company
     * is returned.
     */
    function ajaxFundValuationNotes($investorID) {

        // angellog("ajax fund valuation notes request. ID: " . $investorID);
        $fundVals = array();
        $fundExits = array();

        $no = new angelFundNote();
        $no->connect();
        $r = $no->getRows("fundnotestable", "investorID=$investorID and (type='valuation update' or type='exit prediction') ORDER BY noteDate DESC, last_update DESC");

        if ($r == NULL) {
            echo("AC-ERR='accessing notes failed'");
            die();
        }

        $rows = array();
        /*
         * go through each row, only returning most recent valuation update (the first) and exit prediction
         */
        while($row = mysqli_fetch_assoc($r)) {
            $fundID = $row["fundID"];
            $type = $row["type"];
            if ($type == "valuation update" and !array_key_exists($fundID, $fundVals)) {
                $rows[] = $row;
                $fundVals[$fundID] = $row;
            } elseif ($type == "exit prediction" and !array_key_exists($fundID, $fundExits)) {
                $rows[] = $row;
                $fundExits[$fundID] = $row;
            }
        }

        echo json_encode($rows);

    }

    /*******************
     * This method returns a company database via json for the input investorID
     */
    function ajaxCompanies($investorID) {

        angelGetMyCompanies($rows, $investorID);

        echo json_encode($rows);

    }

    /*
     * Post contains a file to read to  for the input investor ($id). Show results of import.
     */
    function importInvestmentFile($id) {

        $fileResults = array();
        $skippedCount = 0;
        $addedCount = 0;

        $fileName = $_FILES['csvfile']['name'];
        $tfileName = $_FILES['csvfile']['tmp_name'];
        $fileType = pathinfo($fileName,PATHINFO_EXTENSION);

        if (array_key_exists("uploadSkip", $_POST))
            $skipping = true;
        else
            $skipping = false;

        // angeldie("importFile: $id $fileName $tfileName ");    // debug

        // Check file size: nothing bigger than 100k allowed.
        if ($_FILES["csvfile"]["size"] > 100000) {
            angeldie("Sorry, your file is too large.");
        }
        // Only allow CSV
        if($fileType != "csv") {
            angeldie("Sorry, your file must be a csv.");
        }
        /*
         * open the file and import the data
         */
        $file = fopen($tfileName, "r") or angeldie("import investments: unable to open uploaded file!");

        $fileHeaderValues = array("date","company","prefshares","commshares","amount","round","prefcert","commcert","price","type","valuation","status", "notes","batch","converted","qsb");
        $angelColValues = array("inv_date","company_id","prefShares","commonShares","invest_amount","round","prefCertNum","commCertNum","price_paid","type","prevaluation","status", "notes","YCbatch","converted","qsb");
        $requiredHeaders = array("date","company","amount","status");

        $headerMap = $this->createHeaderMap($file, $fileHeaderValues, $angelColValues, $requiredHeaders );   // first, map the file headers to our angelinvest columns
        // angeldie("here: " . serialize($headerMap));
        if ($headerMap == NULL) {
            /*
             * probably means required fields are not there, or something else bad happened. this->error has some info.
             */
            angelerror("Importing Investments failed: " . $this->error);
            return;
        }

        /*
         * insert each entry into the angelinvest table
         *    note - we are doing a bad job of handling errors, dups, etc. and need to figure out how to communicate these
         *       probably send an error with info back and then return to import page and display the errors rather than to
         *       the investment page.
         *
         *      We now return an error in this->error from csvlineTOinvest, which is better!
         */
        $lineNo = 0;    // offset correctly later - had to search for header
        $questionableMatches = "";            // companies which might not be matches placed here.
        $qMatch = "";
        while ($line = fgetcsv($file)) {
            $lineNo++;
            $skipThisEntry = false;
            $invest = $this->csvlineTOinvest($id, $line, $headerMap, $qMatch);
            $displayNo = $lineNo+1;    // lineNo starts at 0, show line number starting at 1.
            if ($invest) {
                $invest->investor_id = $id;
                /*
                 * If skipping is set, check if there is a duplicate entry and ignore it.
                 */
                if ($skipping) {
                    $foundInv = $invest->findMatchingInvest();
                    if ($foundInv != null) {
                       $skipThisEntry = true;
                    }


                }
                if (! $skipThisEntry) {
                    if ($qMatch != "")
                        $questionableMatches .= " " . $qMatch;
                    $invest->insertRow();    // Add new row to DB
                    $fileResults[$lineNo] = "[Line $displayNo] Added($id): " . implode(" ", $line);
                    $addedCount++;
                } else {
                    $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Skipped (duplicate): " . implode(" ", $line) . "</span>";
                    $skippedCount++;
                }
            } else {
                $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Not Added: " . implode(" ", $line) . ": " .
                                                            "$this->error" . "</span>";
                $skippedCount++;
            }
        }

        $body = $this->view->ImportResultsBody($addedCount, $skippedCount, $fileResults, $questionableMatches);
        angellog("Imported investments: (added,skipped) " . angelParens($addedCount, $skippedCount));
        $this->view->outputHTML($body);
    }


  /*
  * Post contains a file to read to  for the input investor ($id). Show results of import.
  */
    function importPayoutFile($id) {

        $fileResults = array();
        $skippedCount = 0;
        $addedCount = 0;

        $fileName = $_FILES['csvfile']['name'];
        $tfileName = $_FILES['csvfile']['tmp_name'];
        $fileType = pathinfo($fileName,PATHINFO_EXTENSION);

        /*
         * we've decided that the idea of skipping distributions is a terrible one since sometimes
         * you'll get multiple identical distributions the same day. I have! For now, we are never
         * skipping.
         */
        if (array_key_exists("uploadSkip", $_POST))
            $skipping = false;
        else
            $skipping = false;

        //angeldie("importPayoutFile: $id $fileName $tfileName ");    // debug

        // Check file size: nothing bigger than 100k allowed.
        if ($_FILES["csvfile"]["size"] > 100000) {
            angeldie("Sorry, your file is too large.");
        }
        // Only allow CSV
        if($fileType != "csv") {
            angeldie("Sorry, your file must be a csv.");
        }
        /*
         * open the file and import the data
         */
        $file = fopen($tfileName, "r") or angeldie("Unable to open uploaded file!");


        $fileHeaderValues = array("date","company","type","cash","shares","symbol","price","stock name","distValue","valuation","status", "notes");
        $angelColValues = array("payDate","companyID","type","cash","shares","symbol","sharePrice","x","y","z","zz","notes");
        $requiredHeaders = array("date","company","type","cash");

        $headerMap = $this->createHeaderMap($file, $fileHeaderValues, $angelColValues,$requiredHeaders );   // first, map the file headers to our payouttable columns
        if ($headerMap == NULL) {
            /*
             * probably means required fields are not there, or something else bad happened. this->error has some info.
             */
            angelerror($this->error);
            return;
        }


        /*
         * insert each entry into the payout table
         *    note - we are doing a bad job of handling errors, dups, etc. and need to figure out how to communicate these
         *       probably send an error with info back and then return to import page and display the errors rather than to
         *       the investment page.
         *
         *  spend more time with the above, but we've improved things. $this->error holds an error returned
         *  from csvlinetopayout if things go badly.
         */
        $lineNo = 0;    // offset correctly later - had to search for header
        while ($line = fgetcsv($file)) {
            $lineNo++;
            $skipThisEntry = false;
            $pay = $this->csvlineTOpayout($line, $headerMap, $id);
            $displayNo = $lineNo+1;    // lineNo starts at 0, show line number starting at 1.
            if ($pay) {
                $pay->investorID = $id;
                if ($skipping) {
                    $foundPay = $pay->findMatchingPayout();
                    if ($foundPay != null) {
                        $skipThisEntry = true;
                    }
                }
                if (! $skipThisEntry) {

                    //  angeldie(serialize($pay));  // debug

                    $pay->insertRow();
                    $fileResults[$lineNo] = "[Line $displayNo] Added($id): " . implode(" ", $line);
                    $addedCount++;
                } else {
                    $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Skipped (duplicate): " . implode(" ", $line) . "</span>";
                    $skippedCount++;
                }
            } else {
                $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Not Added: " . implode(" ", $line) . ": " .
                                                                "$this->error" . "</span>";
                $skippedCount++;
            }
        }

        $body = $this->view->ImportResultsBody($addedCount, $skippedCount, $fileResults, "");
        angellog("Imported payouts: (added,skipped) " . angelParens($addedCount, $skippedCount));
        $this->view->outputHTML($body);
    }

 /*
  * Post contains a file to read to  for the input investor ($id). Show results of import.
  */
    function importFundInvestmentFile($id) {

        $fileResults = array();
        $skippedCount = 0;
        $addedCount = 0;

        $fileName = $_FILES['csvfile']['name'];
        $tfileName = $_FILES['csvfile']['tmp_name'];
        $fileType = pathinfo($fileName,PATHINFO_EXTENSION);

        if (array_key_exists("uploadSkip", $_POST))
            $skipping = true;
        else
            $skipping = false;

        //angeldie("importFile: $id $fileName $tfileName ");    // debug

        // Check file size: nothing bigger than 100k allowed.
        if ($_FILES["csvfile"]["size"] > 100000) {
            angeldie("Sorry, your fund investment file is too large.");
        }
        // Only allow CSV
        if($fileType != "csv") {
            angeldie("Sorry, your fund investment file must be a csv.");
        }
        /*
         * open the file and import the data
         */
        $file = fopen($tfileName, "r") or angeldie("import fund investments: unable to open uploaded file!");

        $fileHeaderValues = array("date","fund","type","amount","notes");
        $angelColValues = array("invDate","fundID","type","investAmount", "notes");
        $requiredHeaders = array("date","fund","amount","type");

        $headerMap = $this->createHeaderMap($file, $fileHeaderValues, $angelColValues, $requiredHeaders );   // first, map the file headers to our angelinvest columns
        // angeldie("importfundinvestmentfile: " . serialize($headerMap));
        if ($headerMap == NULL) {
            /*
             * probably means required fields are not there, or something else bad happened. this->error has some info.
             */
            angelerror("Importing Fund Investments failed: " . $this->error);
            return;
        }

        /*
         * insert each entry into the angelinvest table
         *    note - we are doing a bad job of handling errors, dups, etc. and need to figure out how to communicate these
         *       probably send an error with info back and then return to import page and display the errors rather than to
         *       the investment page.
         *
         *      We now return an error in this->error from csvlineTOinvest, which is better!
         */
        $lineNo = 0;    // offset correctly later - had to search for header
        $questionableMatches = "";            // companies which might not be matches placed here.
        $qMatch = "";
        while ($line = fgetcsv($file)) {
            $lineNo++;
            $skipThisEntry = false;
            $invest = $this->csvlineTOfundinvest($id, $line, $headerMap, $qMatch);
            $displayNo = $lineNo+1;    // lineNo starts at 0, show line number starting at 1.
            if ($invest) {
                $invest->investorID = $id;
                /*
                 * If skipping is set, check if there is a duplicate entry and ignore it.
                 */
                if ($skipping) {
                    $foundInv = $invest->findMatchingInvest();
                    if ($foundInv != null) {
                        $skipThisEntry = true;
                    }


                }
                if (! $skipThisEntry) {
                    if ($qMatch != "")
                        $questionableMatches .= " " . $qMatch;
                    $invest->insertRow();    // Add new row to DB
                    $fileResults[$lineNo] = "[Line $displayNo] Added($id): " . implode(" ", $line);
                    $addedCount++;
                } else {
                    $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Skipped (duplicate): " . implode(" ", $line) . "</span>";
                    $skippedCount++;
                }
            } else {
                $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Not Added: " . implode(" ", $line) . ": " .
                    "$this->error" . "</span>";
                $skippedCount++;
            }
        }

        $body = $this->view->ImportResultsBody($addedCount, $skippedCount, $fileResults, $questionableMatches);
        angellog("Imported investments: (added,skipped) " . angelParens($addedCount, $skippedCount));
        $this->view->outputHTML($body);
    }   // importfundinvestmentfile


    /*
    * Post contains a file to read to  for the input investor ($id). Show results of import.
    */
    function importFundPayoutFile($id) {

        $fileResults = array();
        $skippedCount = 0;
        $addedCount = 0;

        $fileName = $_FILES['csvfile']['name'];
        $tfileName = $_FILES['csvfile']['tmp_name'];
        $fileType = pathinfo($fileName,PATHINFO_EXTENSION);

        /*
         * we've decided that the idea of skipping distributions is a terrible one since sometimes
         * you'll get multiple identical distributions the same day. I have! For now, we are never
         * skipping.
         */
        if (array_key_exists("uploadSkip", $_POST))
            $skipping = false;
        else
            $skipping = false;

        //angeldie("importFundPayoutFile: $id $fileName $tfileName ");    // debug

        // Check file size: nothing bigger than 100k allowed.
        if ($_FILES["csvfile"]["size"] > 100000) {
            angeldie("Sorry, your file is too large.");
        }
        // Only allow CSV
        if($fileType != "csv") {
            angeldie("Sorry, your file must be a csv.");
        }
        /*
         * open the file and import the data
         */
        $file = fopen($tfileName, "r") or angeldie("Unable to open uploaded file!");


        $fileHeaderValues = array("date","fund","type","cash","shares","symbol","price","stock name","distValue","valuation","status", "notes");
        $angelColValues = array("payDate","companyID","type","cash","shares","symbol","sharePrice","x","y","z","zz","notes");
        $requiredHeaders = array("date","fund","type","cash");

        $headerMap = $this->createHeaderMap($file, $fileHeaderValues, $angelColValues,$requiredHeaders );   // first, map the file headers to our payouttable columns
        if ($headerMap == NULL) {
            /*
             * probably means required fields are not there, or something else bad happened. this->error has some info.
             */
            angelerror($this->error);
            return;
        }


        /*
         * insert each entry into the payout table
         *    note - we are doing a bad job of handling errors, dups, etc. and need to figure out how to communicate these
         *       probably send an error with info back and then return to import page and display the errors rather than to
         *       the investment page.
         *
         *  spend more time with the above, but we've improved things. $this->error holds an error returned
         *  from csvlinetopayout if things go badly.
         */
        $lineNo = 0;    // offset correctly later - had to search for header
        while ($line = fgetcsv($file)) {
            $lineNo++;
            $skipThisEntry = false;
            $pay = $this->csvlineTOFundpayout($line, $headerMap, $id);
            $displayNo = $lineNo+1;    // lineNo starts at 0, show line number starting at 1.
            if ($pay) {
                $pay->investorID = $id;
                $pay->changeTable("fundpayouttable");
                if ($skipping) {
                    $foundPay = $pay->findMatchingPayout();
                    if ($foundPay != null) {
                        $skipThisEntry = true;
                    }
                }
                if (! $skipThisEntry) {

                    //  angeldie(serialize($pay));  // debug

                    $pay->insertRow();
                    $fileResults[$lineNo] = "[Line $displayNo] Added($id): " . implode(" ", $line);
                    $addedCount++;
                } else {
                    $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Skipped (duplicate): " . implode(" ", $line) . "</span>";
                    $skippedCount++;
                }
            } else {
                $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Not Added: " . implode(" ", $line) . ": " .
                    "$this->error" . "</span>";
                $skippedCount++;
            }
        }

        $body = $this->view->ImportResultsBody($addedCount, $skippedCount, $fileResults, "");
        angellog("Imported payouts: (added,skipped) " . angelParens($addedCount, $skippedCount));
        $this->view->outputHTML($body);
    }   // importfundpayoutfile

    /*
     * Post contains a file to read to  for the input investor ($id). Show results of import.
     */
    function importYCFile($id) {

        // angeldie("importYCFile");

        $fileResults = array();
        $skippedCount = 0;
        $addedCount = 0;

        $fileName = $_FILES['csvfile']['name'];
        $tfileName = $_FILES['csvfile']['tmp_name'];
        $fileType = pathinfo($fileName,PATHINFO_EXTENSION);

        if (array_key_exists("uploadSkip", $_POST))
            $skipping = true;
        else
            $skipping = false;

        // angeldie("importFile: $id $fileName $tfileName ");    // debug

        // Check file size: nothing bigger than 100k allowed.
        if ($_FILES["csvfile"]["size"] > 100000) {
            angeldie("Sorry, your file is too large.");
        }
        // Only allow CSV
        if($fileType != "csv") {
            angeldie("Sorry, your file must be a csv.");
        }
        /*
         * open the file and import the data
         */
        $file = fopen($tfileName, "r") or angeldie("Unable to open uploaded file - Damn!");

        $fileHeaderValues = array("id","name","batch", "description","sector","status");
        $angelColValues = array("YCID","name","YCbatch","shortdesc","sector","status");
        $requiredHeaders = array("id","name","batch","description","sector","status");

        $headerMap = $this->createHeaderMap($file, $fileHeaderValues, $angelColValues, $requiredHeaders );   // first, map the file headers to our angelinvest columns
        // angeldie("here: " . serialize($headerMap));
        if ($headerMap == NULL) {
            /*
             * probably means required fields are not there, or something else bad happened. this->error has some info.
             */
            angelerror("Importing YC File: " . $this->error);
            return;
        }

        /*
         * insert each entry into the companytable table
         *    note - we are doing a bad job of handling errors, dups, etc. and need to figure out how to communicate these
         *       probably send an error with info back and then return to import page and display the errors rather than to
         *       the investment page.
         */
        $lineNo = 0;    // offset correctly later - had to search for header
        while ($line = fgetcsv($file)) {
            $lineNo++;
            $skipThisEntry = false;
            $co = $this->csvlineTOcompany($line, $headerMap);
            $displayNo = $lineNo+1;    // lineNo starts at 0, show line number starting at 1.
            if ($co) {
                $co->creatorID = $id;
                /*
                 * If skipping is set, check if there is a duplicate entry and ignore it.
                 */
                if ($skipping) {
                    $foundCo = $co->findMatchingCompany();
                    if ($foundCo != null) {
                        // angeldie("(co.name, foundCo.name)" . angelParens($co->name, $foundCo->name));    // debug
                        $skipThisEntry = true;
                    }


                }
                if (! $skipThisEntry) {
                    $co->insertRow();    // Add new row to DB
                    $fileResults[$lineNo] = "[Line $displayNo] Added($id): " . implode(" ", $line);
                    $addedCount++;
                } else {
                    $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Skipped (duplicate): " . implode(" ", $line) . "</span>";
                    $skippedCount++;
                }
            } else {
                $fileResults[$lineNo] = "<span style='color:red'>[Line $displayNo] Not Added: " . implode(" ", $line) . "</span>";
                $skippedCount++;
            }
        }

        $body = $this->view->ImportResultsBody($addedCount, $skippedCount, $fileResults, "");
        angellog("Imported YC file: (added,skipped) " . angelParens($addedCount, $skippedCount));
        $this->view->outputHTML($body);
    }


    /*************************************************************************************************************
     * create and return an associative map of the file in the format: [col number in file] [fieldname from table]
     * @param $f                    -- input file handle
     * @param $fileHeaderValues     -- comma separated list of possible column headers
     * @param $angelColValues       -- comma separated list of matching column names in table
     * @return array|null
     */

    function createHeaderMap($f, $fileHeaderValues, $angelColValues, $requiredHeaders ) {
        /*
         * first, find the header
         */
        $colMap = array_combine($fileHeaderValues, $angelColValues);

        $map = array();
        $found = false;

        while (! $found) {
            $csvline = fgetcsv($f);
            $csvline = $this->cleanupCSVline($csvline);
            // echo "csvline -->" . serialize($csvline) . "<br>";
            if (!$csvline)  // eof
                break;
            foreach ($csvline as $ent) {
                // echo htmlspecialchars($ent, ENT_QUOTES, 'UTF-8') ."[" . $ent[0] . "]<br>";
                $ent = strtolower($ent);
                if ($ent == "date" or $ent == "id") {
                    $found = true;
                }
            }
        }

        if (! $found) {           // nothing useful in the file
            $this->error = "No properly formatted CSV lines Found. This may mean you don't have a 'Date' column
                            or your file's lines are not delimited with line feeds";
            return(null);
        }


        foreach (array_keys($colMap) as $h) {
            $index = $this->findMatchingCol($csvline, $h);
            if ($index >= 0)
                $map[(string)$index] = $colMap[$h];
        }
        //angeldie("createmap: line: " . serialize($csvline) . " ****** map: " . serialize($map));   // debug


        $this->error = $this->checkHeaderMap($csvline, $requiredHeaders);
        if ($this->error == "")
            return($map);
        else
            return(NULL);


    }
    function cleanupCSVline($l) {

        $len = count($l);
        // angeldie("cleanupCSVline: " . angelParens($len, serialize($l)));
        for ($i=0;$i < $len; $i++) {
            $l[$i] = preg_replace('/[[:^print:]]/', '', trim($l[$i]));
        }
        return($l);
    }

    function checkHeaderMap($csvline, $requiredHeaders) {

        // angeldie("here: " . serialize($csvline) . "||" . serialize($requiredHeaders));   // debug

        $missingHeaders = "";

        $needComma = false;
        foreach ($requiredHeaders as $hdr) {
            $hdr = strtolower($hdr);
            $index = $this->findMatchingCol($csvline, $hdr);
            if ($index >= 0)
                continue;
            else {
                if ($needComma)
                    $missingHeaders .= ",$hdr";
                else {
                    $missingHeaders .= "$hdr";
                    $needComma = true;
                }
            }
        }
        if ($missingHeaders == "")
            return("");
        else
            return("The following required header(s) are missing: " . $missingHeaders);
    }

    function findMatchingCol($line, $col) {
        $index = 0;
        foreach ($line as $ent) {
            $ent = strtolower($ent);
            $col = strtolower($col);
            if ($ent == $col)
                return($index);
            $index++;
        }
        return(-1); /* no matching column found */
    }

    /*
     * convert a csvline to an invest structure using map
     */
    function csvlineTOinvest($investorID, $line, $map, &$questionableMatch) {
        $inv = new angelInvest();

        // angeldie("csvlineTOinvest - map: " . serialize($map) . "line: " . serialize($line)); // debug
        /*
         * Here we grab each value from the line and put it in the new angelinvest object
         */
        // angeldie("csv: " . serialize($map) . "||||||" . serialize($line));
        foreach (array_keys($map) as $index) {
            $val = $line[(int)$index];
            $val = str_replace('$', '', $val);
            $val = str_replace(',', '', $val);
            if ($map[$index] == "inv_date") {
                if (substr_count($val, "/") != 2 and substr_count($val, "-") != 2) {
                    $this->error = "no legal date found";
                    return(null);   // gonna be harsh - if not in a recognized date format skip the line
                }

                $inv->{$map[$index]} = date('Y-m-d', strtotime(str_replace('-', '/', $val)));
            } else
                $inv->{$map[$index]} = $val;

        }
        $inv->round = strtolower($inv->round);
        $inv->type = strtolower($inv->type);
        $inv->status = strtolower($inv->status);

        //angeldie("csvlineTOinvest: " . serialize($inv));  // debug
        if (is_numeric($inv->company_id)) {
            $this->error = "$inv->company_id does not seem to be a legal company name";
            return(null);
        } else {
            // company name - let's look it up and add if not there.  If there, use its id.

            $co = new angelCompany();
            $conn = $co->connect();
            $co->name = $inv->company_id;
            $co->kind = "startup";
            $co->status = $inv->status;
            $inv->YCbatch = strtolower($inv->YCbatch);
            $co->YCbatch = $inv->YCbatch;
            $co->creatorID = $investorID;
            $questionableMatch = "";
            $coFound = $this->findFuzzilyMatchingCompany($co, $questionableMatch);
            if ($coFound == null) {
                angellog("Import Invest: no match found for company: " . $co->name . " adding to company DB");
                $inv->company_id = $co->insertRow();     // insert new company into DB
            } else {
                $inv->company_id = $coFound;
            }


            if ($inv->round != "")
                $inv = $this->parseRound($inv);

            if (is_string($inv->qsb)) {
                $inv->qsb = strtolower($inv->qsb);
                $inv->qsb = ($inv->qsb == "y" or $inv->qsb == "yes") ? true : false;
            }


            if (is_string($inv->converted)) {
                $inv->converted = strtolower($inv->converted);
                $inv->converted = ($inv->converted == "y" or $inv->converted == "yes") ? true : false;
            }
            if ($inv->type == "note" or $inv->type == "safe") {
                $inv->cap = $inv->prevaluation;
                $inv->prevaluation = 0;
            }

            // angeldie(angelParens(serialize($line), serialize($inv)));


            return($inv);
        }
    }       // csvlinetoinvest

    /*
      * convert a csvline to an invest structure using map
      */
    function csvlineTOfundinvest($investorID, $line, $map, &$questionableMatch) {
        $inv = new angelFundInvest();

        // angeldie("csvlineTOfundinvest - map: " . serialize($map) . "line: " . serialize($line)); // debug
        /*
         * Here we grab each value from the line and put it in the new angelinvest object
         */
        // angeldie("csv: " . serialize($map) . "||||||" . serialize($line));
        foreach (array_keys($map) as $index) {
            $val = $line[(int)$index];
            $val = str_replace('$', '', $val);
            $val = str_replace(',', '', $val);
            if ($map[$index] == "invDate") {
                if (substr_count($val, "/") != 2 and substr_count($val, "-") != 2) {
                    $this->error = "no legal date found";
                    return(null);   // gonna be harsh - if not in a recognized date format skip the line
                }

                $inv->{$map[$index]} = date('Y-m-d', strtotime(str_replace('-', '/', $val)));
            } else
                $inv->{$map[$index]} = $val;

        }
        $inv->type = strtolower($inv->type);
        $inv->investorID = $investorID;

        //angeldie("csvlineTOfundinvest: " . serialize($inv));  // debug
        if (is_numeric($inv->fundID)) {
            $this->error = "$inv->fundID does not seem to be a legal fund name";
            return(null);
        } else {
            // fund name - let's look it up and add if not there.  If there, use its id.

            $fund = new angelFund();
            $conn = $fund->connect();
            $fund->name = $inv->fundID;
            $fund->status = "active";
            $fund->creatorID = $investorID;
            $questionableMatch = "";
            /*
             * I'd like to do a fuzzy match to look up the fund, but this seems like a bad idea after trying
             * it since multiple different funds have almost the same name.
             */
            $fuFound = $this->findFuzzilyMatchingFund($fund, $questionableMatch);
            // $fuFound = $fund->findMatchingFund(true);       // boolean indicates exact match
            if ($fuFound == null) {
                angellog("Import Invest: no match found for fund: " . $fund->name . " adding to company DB");
                //echo("Import Invest: no match found for fund: " . $fund->name . " adding to company DB<br/>");
                $inv->fundID = $fund->insertRow();     // insert new company into DB
            } else {
                $inv->fundID = $fuFound;
            }
            /*
             * before we return our new investment, make sure there is a matching commitment if it isn't one
             */
            if ($inv->type == "invest") {
                $comm = $inv->findMatchingCommit();
                if ($comm == null) {
                    $this->error = "No matching commitment found for this fund investment.";
                    return (null);
                }


            }

            // angeldie(angelParens(serialize($line), serialize($inv)));


            return($inv);
        }
    }       // csvlinetofundinvest

    /*
     * see if a company matches the input company name with an appropriately fuzzy search. if
     * found return it's id, otherwise null
     */
    function findFuzzilyMatchingCompany($co, &$questionableMatch) {

        $cidTOname = array();

        $co->createMap($cidTOname, "companytable", "id", "name");
        /*
         * First Pass - loop through all companies and find most similar match
         */
        $questionableMatch = "";    // return of a company match which may be wrong (we are fuzzy!)
        $highestPercent = 0;
        foreach(array_keys($cidTOname) as $id) {
            $name = $this->cleanCompanyName($cidTOname[$id]);
            $coName = $this->cleanCompanyName($co->name);
            similar_text($name, $coName, $percent);

            if ($percent > $highestPercent) {
                $highestID = $id;
                $highestPercent = $percent;
            }
        }
        //angeldie("percent= " . $highestPercent . "(name,co->name):" . angelParens($cidTOname[$highestID], $co->name));   // debug


        $id = $highestID;
        $name = $cidTOname[$id];
        $name = $this->cleanCompanyName($name);
        $coName = $this->cleanCompanyName($co->name);
        similar_text($name, $coName, $percent);
        $soundex1 = soundex($name);
        $soundex2 = soundex($coName);
        $meta1 = metaphone($name, 5);
        $meta2 = metaphone($coName, 5);
        $lev = levenshtein($name, $coName, 1, 3, 1);
        /*
         *  Debug code to see what all this magic matching does.
         *
        echo ("(new name, matched name) (percent,lev) (soundex1, soundex2) (meta1, meta2)" . angelParens($coName,$name) .
            angelParens($percent, $lev) . angelParens($soundex1, $soundex2) . angelParens($meta1, $meta2) . "<br />");
        return(null);*/

        //echo("(" . $name . "," . $coName . ") -- " . $percent . "(" . $soundex1 . "," . $soundex2 . ")" . " lev: " . $lev . "<br />");
        /*
         * Our fuzzy algorithm, such as it is (all thresholds are pseudo-arbitrary)
         *
         *     Match if:    - similar text thinks there is a > 70% match                             AND
         *                  - the soundex codes match for at least the first 3 (of 4) characters     AND
         *                  - the levenshtein difference minus the absolute value of the difference
         *                    in lengths of the two strings < 10 (arbitrarily chosen)
         *            OR
         *
         *                  - the soundex codes match                                                AND
         *                  - the levenshtein difference minus the absolute value of the difference
         *                    in lengths of the two strings < 8 (arbitrarily chosen)                 AND
         *                  - the lengths of the two strings differ by more than 2
         *
         * These cases have forced us to move percent match over 70. To be seen if we miss more.
         *
         * (Wevorce,Weave) -- 66.666666666667(W162,W100) lev: 4
         * (Fundly,Fanduel) -- 61.538461538462(F534,F534) lev: 5
         *
         */
        if (($percent > 75 and
            substr($soundex1,0,2) == substr($soundex2,0,2) and
            abs($lev - abs(strlen($name) - strlen($coName))) < 10)
          or ((substr($soundex1,0,4) == substr($soundex2,0,4) and
                $this->metaphoneCompare($meta1, $meta2) and
                abs($lev - abs(strlen($name) - strlen($coName))) < 8) and
                abs(strlen($name) - strlen($coName)) > 2)

        ) {
            /*
             * we found a matching company.return the id.
             */
            if ($percent < 80) {

                angellog("Fuzzy Company matched: (new name, matched name) (percent,lev) (soundex1, soundex2)" . angelParens($coName,$name) .
                    angelParens($percent, $lev) . angelParens($soundex1, $soundex2));

               /* echo ("(new name, matched name) (percent,lev) (soundex1, soundex2) (meta1, meta2)" . angelParens($coName,$name) .
                    angelParens($percent, $lev) . angelParens($soundex1, $soundex2) . angelParens($meta1, $meta2) . "<br />"); */
               /**angeldie("Fuzzy Company matched: (new name, matched name) (percent,lev) (soundex1, soundex2) (meta1, meta2)" . angelParens($coName,$name) .
                    angelParens($percent, $lev) . angelParens($soundex1, $soundex2) . angelParens($meta1, $meta2));*/

                $questionableMatch = angelParens($coName,$name);

            }
            //echo("(" . $cidTOname[$id] . "," . $co->name . ") -- " . $percent . "(" . $soundex1 . "," . $soundex2 . ")" . " lev: " . $lev . "<br />");
            return($id);
        }

        // angeldie("all done!!");      // debug

        return(null);

    }
    /*
     * remove Inc., Corp, Incorperated, Corporation, and anything in parens from input name
     */
    function cleanCompanyName($name) {

        $name = strtolower(trim($name));            // remove whitespace and lowercase everything
        $name = str_replace(".", " ", $name);           // remove periods
        $name = str_replace(",", " ", $name);           // remove commas
        $name = $name . " ";                            // put space at the end to make sure we match below
        $name = str_replace(" inc ", " ", $name);
        $name = str_replace(" llc ", " ", $name);
        $name = str_replace(" lp ", " ", $name);
        $name = str_replace(" l p ", " ", $name);
        $name = str_replace(" incorporated ", "", $name);
        $name = str_replace(" corp ", "", $name);
        $name = str_replace(" corporation ", "", $name);



        $name = str_replace(" ", "", $name);            // finally remove all spaces

        return($name);

    }
    /*
     * compare to metaphone results and decide if they warrant assuming they are for the same company
     */
    function metaphoneCompare($meta1, $meta2) {

        $len1 = strlen($meta1);
        $len2 = strlen($meta2);

        $len = $len1 > $len2 ? $len2 : $len1;
        /*
         * case 1: if one metaphone is only 2 in length and they aren't both 2 in length, then this is a fail.
         * case 2: otherwise, if the two are equal for the first 3 chars we're good to go.
         */
        if ($len < 3 and $len2 != $len1)
            return(false);
        else
            $len = 3;

        if (substr($meta1,0,$len) == substr($meta2,0,$len))
            return(true);
        else
            return(false);

    }

    /*
   * see if a company matches the input company name with an appropriately fuzzy search. if
   * found return it's id, otherwise null
   */
    function findFuzzilyMatchingFund($fu, &$questionableMatch) {

        $fidTOname = array();

        $fu->createMap($fidTOname, "fundtable", "id", "name");
        /*
         * First Pass - loop through all funds and find most similar match
         */
        $questionableMatch = "";    // return of a fund match which may be wrong (we are fuzzy!)
        $highestPercent = 0;
        foreach(array_keys($fidTOname) as $id) {
            $name = $this->cleanCompanyName($fidTOname[$id]);
            $fuName = $this->cleanCompanyName($fu->name);
            similar_text($name, $fuName, $percent);
            // echo ("percent= " . $percent . "(name,fu->name):" . angelParens($name, $fuName) . "<br/>");   // debug

            if ($percent > $highestPercent) {
                $highestID = $id;
                $highestPercent = $percent;
            }
        }
        // echo ("highestpercent= " . $highestPercent . "(name,fu->name):" . angelParens($fidTOname[$highestID], $fu->name) . "<br/>");   // debug


        if ($highestPercent == 100)
            return($highestID);
        else {
            if ($highestPercent > 98) {
                $name = $fidTOname[$highestID];
                $questionableMatch = angelParens($fu->name,$name);
                return($highestID);
            } else
                return(null);
        }
        /*
         * I think I can delete the rest of this!!
         */
        $id = $highestID;
        $name = $fidTOname[$id];
        $name = $this->cleanCompanyName($name);
        $fuName = $this->cleanCompanyName($fu->name);
        similar_text($name, $fuName, $percent);
        $soundex1 = soundex($name);
        $soundex2 = soundex($fuName);
        $meta1 = metaphone($name, 5);
        $meta2 = metaphone($fuName, 5);
        $lev = levenshtein($name, $fuName, 1, 3, 1);
        /*
         *  Debug code to see what all this magic matching does.
         *
        echo ("(new name, matched name) (percent,lev) (soundex1, soundex2) (meta1, meta2)" . angelParens($coName,$name) .
            angelParens($percent, $lev) . angelParens($soundex1, $soundex2) . angelParens($meta1, $meta2) . "<br />");
        return(null);*/

        //echo("(" . $name . "," . $coName . ") -- " . $percent . "(" . $soundex1 . "," . $soundex2 . ")" . " lev: " . $lev . "<br />");
        /*
         * Our fuzzy algorithm, such as it is (all thresholds are pseudo-arbitrary)
         *
         *     Match if:    - similar text thinks there is a > 85% match                             AND
         *                  - the soundex codes match for at least the first 3 (of 4) characters     AND
         *                  - the levenshtein difference minus the absolute value of the difference
         *                    in lengths of the two strings < 10 (arbitrarily chosen)
         *            OR
         *
         *                  - the soundex codes match                                                AND
         *                  - the levenshtein difference minus the absolute value of the difference
         *                    in lengths of the two strings < 3 (arbitrarily chosen)                 AND
         *                  - the lengths of the two strings differ by more than 2
         *
         * These cases have forced us to move percent match over 70. To be seen if we miss more.
         *
         * (Wevorce,Weave) -- 66.666666666667(W162,W100) lev: 4
         * (Fundly,Fanduel) -- 61.538461538462(F534,F534) lev: 5
         *
         */
        if (($percent > 93 and
                substr($soundex1,0,3) == substr($soundex2,0,3) and
                abs($lev - abs(strlen($name) - strlen($fuName))) < 10)
            or ((substr($soundex1,0,4) == substr($soundex2,0,4) and
                    $this->metaphoneCompare($meta1, $meta2) and
                    abs($lev - abs(strlen($name) - strlen($fuName))) < 3) and
                abs(strlen($name) - strlen($fuName)) > 5)

        ) {
            /*
             * we found a matching company.return the id.
             */
            if ($percent < 98) {

                angellog("Fuzzy Fund matched: (new name, matched name) (percent,lev) (soundex1, soundex2)" . angelParens($fuName,$name) .
                    angelParens($percent, $lev) . angelParens($soundex1, $soundex2));

                /* echo ("(new name, matched name) (percent,lev) (soundex1, soundex2) (meta1, meta2)" . angelParens($coName,$name) .
                     angelParens($percent, $lev) . angelParens($soundex1, $soundex2) . angelParens($meta1, $meta2) . "<br />"); */
                /**angeldie("Fuzzy Company matched: (new name, matched name) (percent,lev) (soundex1, soundex2) (meta1, meta2)" . angelParens($coName,$name) .
                angelParens($percent, $lev) . angelParens($soundex1, $soundex2) . angelParens($meta1, $meta2));*/

                $questionableMatch = angelParens($fuName,$name);

            }
            //echo("(" . $cidTOname[$id] . "," . $co->name . ") -- " . $percent . "(" . $soundex1 . "," . $soundex2 . ")" . " lev: " . $lev . "<br />");
            return($id);
        }

        // angeldie("all done!!");      // debug

        return(null);

    }
 /*
  * convert a csvline to an payout structure using map
  */
    function csvlineTOpayout($line, $map, $investorID) {
        $pay = new angelPayout();
        $this->error = "";

        //angeldie("csvlineTOpayout - map: " . serialize($map) . "line: " . serialize($line)); // debug
        /*
         * Here we grab each value from the line and put it in the new angelinvest object
         */
        foreach (array_keys($map) as $index) {
            $val = $line[(int)$index];
            $val = str_replace('$', '', $val);
            $val = str_replace(',', '', $val);
            if ($map[$index] == "payDate") {
                if (substr_count($val, "/") != 2 and substr_count($val, "-") != 2) {
                    $this->error = "no legal date found";
                    return(null);   // gonna be harsh - if not in a recognized date format skip the line
                }

                $pay->{$map[$index]} = date('Y-m-d', strtotime(str_replace('-', '/', $val)));
            } else
                $pay->{$map[$index]} = $val;

        }
        $pay->type = strtolower($pay->type);

        //angeldie("csvlineTOpayout: " . serialize($inv));  // debug
        if (is_numeric($pay->companyID)) {
            $this->error = "$pay->companyID does not seem to be a legal company name";
            return(null);
        } else {
            /*
             * company name - let's look it up and if not found, skip the line (the payout MUST be
             * tied to a known company investment.
             */
            $qMatch = "";           // we don't pay attention to questionable matches, but maybe we should?
            $co = new angelCompany();
            $conn = $co->connect();
            $co->name = $pay->companyID;
            $coFound = $this->findFuzzilyMatchingCompany($co, $qMatch);
            if ($coFound == null) {
                angellog("Import payout: no match found for company: " . $co->name . " skipping");
                $this->error = "$co->name is not a recognized company";
                return(null);
            } else {
                $pay->companyID = $coFound;
                /*
                 * before we return our new payout, make sure there is a matching investment
                 */
                $inv = new angelInvest();
                $inv->shareconn($conn);
                $rows = $inv->getRows("angelinvests", "investor_id=$investorID and company_id=$coFound");
                if (mysqli_num_rows($rows) == 0) {
                    $this->error = "no investment in company $co->name found matching this distribution";
                    return(null);
                }


            }

            return($pay);
        }
    }       // csvlinetopayout

 /*
 * convert a csvline to a fund payout structure using map
 */
    function csvlineTOFundpayout($line, $map, $investorID) {
        $pay = new angelPayout();
        $this->error = "";

        //angeldie("csvlineTOpayout - map: " . serialize($map) . "line: " . serialize($line)); // debug
        /*
         * Here we grab each value from the line and put it in the new angelinvest object
         */
        foreach (array_keys($map) as $index) {
            $val = $line[(int)$index];
            $val = str_replace('$', '', $val);
            $val = str_replace(',', '', $val);
            if ($map[$index] == "payDate") {
                if (substr_count($val, "/") != 2 and substr_count($val, "-") != 2) {
                    $this->error = "no legal date found";
                    return(null);   // gonna be harsh - if not in a recognized date format skip the line
                }

                $pay->{$map[$index]} = date('Y-m-d', strtotime(str_replace('-', '/', $val)));
            } else
                $pay->{$map[$index]} = $val;

        }
        $pay->type = strtolower($pay->type);

        //angeldie("csvlineTOfundpayout: " . serialize($inv));  // debug
        if (is_numeric($pay->companyID)) {
            $this->error = "$pay->companyID does not seem to be a legal fund name";
            return(null);
        } else {
            /*
             * fund name - let's look it up and if not found, skip the line (the payout MUST be
             * tied to a known company investment.
             */
            $qMatch = "";           // we don't pay attention to questionable matches, but maybe we should?
            $fund = new angelFund();
            $conn = $fund->connect();
            $fund->name = $pay->companyID;
            $fuFound = $this->findFuzzilyMatchingFund($fund, $qMatch);   // fuzzy match works now!
            // $fuFound = $fund->findMatchingFund(true);       // boolean indicates exact match
            if ($fuFound == null) {
                angellog("Import fund payout: no match found for fund: " . $fund->name . " skipping");
                $this->error = "$fund->name is not a recognized fund";
                return(null);
            } else {
                $pay->companyID = $fuFound;
                /*
                 * before we return our new payout, make sure there is a matching investment
                 */
                $inv = new angelInvest();
                $inv->shareconn($conn);
                $rows = $inv->getRows("fundinvests", "investorID=$investorID and fundID=$fuFound");
                if (mysqli_num_rows($rows) == 0) {
                    $this->error = "no investment in fund $fund->name found matching this distribution";
                    return(null);
                }


            }

            return($pay);
        }
    }       // csvlinetofundpayout


    /*
   * convert a csvline to a company structure using map
   */
    function csvlineTOcompany($line, $map) {
        // angeldie("csvlineTOcompany (line, map):" . angelParens(serialize($line), serialize($map)));     // debug
        $co = new angelCompany();

        /*
         * Here we grab each value from the line and put it in the new company object
         */
        foreach (array_keys($map) as $index) {
            $val = $line[(int)$index];
            $val = str_replace('$', '', $val);
            $val = str_replace(',', '', $val);
            $co->{$map[$index]} = $val;

        }
        $co->kind = "startup";
        $co->status = strtolower($co->status);
        $co->YCbatch = strtolower($co->YCbatch);

        return($co);

    }       // csvlinetocompany

    /*
     * If round is in format 3mm cap note,convert,safe or Series A preferred we tease it apart
     */
    function parseRound($inv) {
        // angeldie("parseRound: " . serialize($inv));
        if ($inv->round == "") {
            if ($inv->type == "safe" or $inv->type =="note")
                $inv->round = "seed";   // this will usually be right
            return($inv);
        }

        $round = strtolower($inv->round);
        $roundArr = explode(" ", $round);

        $pos = strpos($roundArr[1], "cap");

        if ($pos !== false) {
            /*
             * found a cap, so it's a convert
             */
            if (sizeof($roundArr) > 2 and $inv->type == "") {
                $pos = strpos($roundArr[2], "note");
                if ($pos !== false)
                    $inv->type = "note";
                else
                    $inv->type = "safe";
            }

            $val = str_replace('$', '', $roundArr[0]);
            $val = str_replace('mm', '', $val);
            $inv->cap = $val*1000000;
            $inv->round = "seed";
            $inv->type = strtolower($inv->type);

            // angeldie("parseRound: " . serialize($inv));

        } else {

            /*
             * see if we can find out if it's an  equity round
             */
            $pos = strpos($roundArr[0], "series");
            if ($pos !== false) {
                if ($roundArr[1] = "a")
                    $inv->round = "series A";
                elseif ($roundArr[1] = "b")
                    $inv->round = "series B";
                elseif ($roundArr[1] = "c")
                    $inv->round = "series C";
                elseif ($roundArr[1] = "d")
                    $inv->round = "series D";

                $inv->type = "equity";
            }


        }

        return($inv);

    }

    /*
     * Post contains conversion info for converting input investment (id) to equity
     */


    function convertInvestment($id) {
        $errorFound = "";
        /*
         * first look up the investment to convert
         */
        $convertInvest = new angelInvest();
        $convertInvest->id = $id;
        $conn = $convertInvest->connect();
        $convertInvest = $convertInvest->getEntry();

        // angeldie(serialize($convertInvest));  // debug

        $newInvest = clone $convertInvest;

        $newInvest->round = 'series A';
        $newInvest->type = "equity";
        $newInvest->convert_id = $id;
        $newInvest->prevaluation = $_POST["valuation"];
        $newInvest->current_valuation = $_POST["valuation"];
        $newInvest->cap = 0;
        $newInvest->discount = 0;
        $newInvest->shares = $_POST["yourSharesForm"];
        $newInvest->total_fdshares = $_POST["postFDSharesForm"];
        $newInvest->price_paid = $_POST["yourPriceForm"];
        $newInvest->price_share = $_POST["postPriceForm"];
        $today = date("Y-m-d");
        $newInvest->notes .= " <span style='color:red'>[Converted from $convertInvest->type on $today]</span>";

        // angeldie(serialize($newInvest));  // debug


        $newID = $newInvest->insertRow();

        // now update next field
        $q =  sprintf("UPDATE angelinvests SET convert_id=$newID WHERE id=%s", $id);
        // angeldie($q);    // debug
        $result = mysqli_query($conn,$q);
        if ($result == NULL)
            angeldie("angelinvests update failed:" . $q );

        // all done!!

        return($errorFound);


    }

    /*
     * Post contains new Investment info for investor ($id). Either brand new or updated (if $updating is true)
     */


    function addOrUpdateInvestment($id, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to investmenttable
         */
        $errorFound = "";

        $inv = new angelInvest();
        $conn = $inv->connect();

        $inv->investor_id = $id;
        $inv->company_id = $_POST['company'];
        $inv->round = $_POST['round'];   // assume good since a selector
        $inv->type = $_POST['type'];     // ditto
        $inv->inv_date = $_POST['date'];     // date selector
        if ($inv->inv_date == "") {
            $errorFound .= "Please enter a date.<cr>";
        } else {
            //angeldie($inv->inv_date);
            $inv->inv_date = date('Y-m-d', strtotime(str_replace('-', '/', $inv->inv_date)));
        }

        $inv->invest_amount = $_POST['invest_amount'];
        $inv->prevaluation = $_POST['prevaluation'];              // this is either a value / cap depending on type
        $inv->prefShares = $_POST['prefShares'];
        $inv->prefSharesSeries = $_POST['prefSharesSeries'];
        $inv->commonShares = $_POST['commonShares'];

        if (array_key_exists('converted', $_POST))
            $inv->converted = true;
        else
            $inv->converted = false;

        $inv->commCertNum = $_POST['commCertNum'];
        $inv->prefCertNum = $_POST['prefCertNum'];

        $inv->certLoc = $_POST['certLoc'];
        $inv->discount = $_POST['discount'];
        $inv->status = 'active';                // we only add or update active investments (I think!!)

        $inv->shares = $inv->prefShares + $inv->commonShares;
        if ($inv->type == "option" or $inv->type == "warrant") {
            $inv->price_paid = $inv->prevaluation;
            $inv->shares = $inv->invest_amount;
        } else
            $inv->price_paid = $inv->invest_amount / $inv->shares;


        $co = new angelCompany();
        $co->shareconn($conn);


        if (!is_numeric($inv->company_id)) {
            /*
             * the company is either new, or not quite an exact match. see if we can find it first
             *
             */
            $companyName = mysqli_escape_string($conn, $inv->company_id );
            $co = new angelCompany();
            $co->name = $companyName;
            $co->connect();
            $co = $co->findMatchingCompany();
            if ($co != NULL) {  // found it!
                $inv->company_id = $co->id;
            } else {
                $q = "INSERT INTO companytable (kind,name) VALUES ('startup','$companyName')";
                $result = mysqli_query($conn,$q);
                if ($result == NULL)
                    angeldie("companytable insert of new startup $inv->company_id failed:" . $q );

                $q = "SELECT id from companytable WHERE name='$companyName'";
                $result = mysqli_query($conn,$q);

                if ($result == NULL)
                    angeldie("companytable select for new company $inv->company_id failed:" . $q );

                $co = mysqli_fetch_object($result, 'angelCompany');
                $inv->company_id = $co->id;
            }
        }

        /*
         * the round/series is either an id referring to a round/series name or a new one.
         */
        $inv->round = angelRound($inv->round, $conn);
        $inv->prefSharesSeries = angelRound($inv->prefSharesSeries, $conn);

        if ($inv->invest_amount == "" or $inv->invest_amount == 0) {
            $errorFound .=  "Please enter an investment amount.<cr>";
        }
        if ($inv->invest_amount > 100000000) {
            $errorFound .=  "Amount ($inv->invest_amount  dollars) is too big - I don't believe it.<cr>";
        }
        if ($inv->prevaluation == "")
            $value = 0;

        if ($inv->prevaluation > 100000000000) {
            $errorFound .= "Value/Cap ($inv->prevaluation dollars) is too big - I don't believe it.<cr>";
        }

        if ($inv->type == 'safe' or $inv->type == "note") {
            $inv->cap = $inv->prevaluation;
            $inv->prevaluation = "";
        } else {
            $inv->cap = "";
        }

        $inv->notes = mysqli_escape_string($conn, $_POST['notes']);
        $inv->status = "active";

        /*
         * The investors come as an array with a single element which is a comma separated list of ids
         * and names (of new investors). getInvestorsIDs expects an array with each element being an id/name
         */

        $x = $_POST['vcs'];
        if ($x[0] != "")
            $inv->investco_ids    = ik12_getInvestorIDs($conn, explode(',', $x[0]), 'vc');

        $x = $_POST['angels'];
        if ($x[0] != "")
            $inv->angel_ids = ik12_getInvestorIDs($conn, explode(',', $x[0]), 'angel');


        if ($errorFound == "") {


            if ($updating) {
                $inv->id = $_POST['update'];     // id of investment to update
                $id = $inv->updateThisRow();
            } else {
                $id = $inv->insertRow();
                /*
                 * add investment to company as well.
                 */
            }

            // angeldie($q);    // debug

        }
        return($errorFound);
    }   // addOrUpdateInvestment


   /*
   * Post contains new payout info for investor ($id). Either brand new or updated (if $updating is true)
   */


    function addOrUpdatePayout($id, $updating) {
        // angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to investmenttable
         */
        $errorFound = "";

        $pay = new angelPayout();
        $conn = $pay->connect();

        $pay->investorID = $id;
        $pay->type = $_POST['type'];         // selector: Cash or Stock
        $pay->payDate = $_POST['date'];     // date selector
        if ($pay->payDate == "") {
            $errorFound .= "Please enter a date.<cr>";
        } else {
            $pay->payDate = date('Y-m-d', strtotime(str_replace('-', '/', $pay->payDate)));
        }

        if ($pay->type == "cash") {
            $pay->cash = $_POST['payoutCash'];
        } else {
            $pay->shares = $_POST['payoutShares'];
            $pay->sharePrice = $_POST['payoutPrice'];
            $pay->symbol = $_POST['payoutSymbol'];
        }

        $pay->basis = $this->payoutBasis($pay, $id);     // get the basis (if any) from _POST

        if (array_key_exists("lastPayout", $_POST))
            $pay->lastPayout = true;


        if (array_key_exists("fund", $_POST)) {
            $pay->changeTable("fundpayouttable");
            $pay->companyID = $_POST['fund'];

        } else {
            $pay->companyID = $_POST['company'];
        }

        if (!is_numeric($pay->companyID)) {
            $errorFound .= "Must be an existing company or fund.<cr>";
        }

        $pay->notes = mysqli_escape_string($conn, $_POST['payoutNotes']);


        if ($errorFound == "") {


            if ($updating) {
                $pay->id = $_POST['update'];     // id of payout to update
                $id = $pay->updateThisRow();
            } else {
                $id = $pay->insertRow();
            }

            $this->updateInvestmentBasis($pay);     // update each investment which is used as basis

            // angeldie($q);    // debug

        } else
            angeldie("addOrUpdatePayout: shouldn't be here, I think:" . $errorFound);

        return($errorFound);
    }   // addOrUpdatePayout

    /*
     * Post contains new fund payment info for investor ($id). Either brand new or updated (if $updating is true)
     */


    function addOrUpdateFundPayment($id, $updating) {
        //angeldie(serialize($_POST));   // debug
        /*
         * Verify all of the input and if all good, add a new row to investmenttable
         */
        $errorFound = "";

        $pay = new angelFundInvest();
        $conn = $pay->connect();

        $pay->investorID = $id;
        $pay->invDate = $_POST['date'];     // date selector
        if ($pay->invDate == "") {
            $errorFound .= "Please enter a date.<cr>";
        } else {
            $pay->invDate = date('Y-m-d', strtotime(str_replace('-', '/', $pay->invDate)));
        }

        $pay->investAmount = $_POST['paymentAmount'];
        $pay->fundID = $_POST['fund'];


        if (!is_numeric($pay->fundID)) {
            $errorFound .= "Must be an existing company or fund.<cr>";
        } else {
            $pay->status = "active";
            $pay->type = "invest";
        }

        $pay->notes = mysqli_escape_string($conn, $_POST['paymentNotes']);


        if ($errorFound == "") {
            if ($updating) {
                $pay->id = $updating;
                $id = $pay->updateThisRow();
            } else {
                $id = $pay->insertRow();
            }

            // angeldie($q);    // debug

        } else
            angeldie("addOrUpdateFundPayment: shouldn't be here, I think:" . $errorFound);

        return($errorFound);
    }   // addOrUpdateFundPayment


    /*
     * return the query string formatted basis for the input payout (investID=amount&investID=amount)
     */
    function payoutBasis($pay, $investorID) {
        $basis = "";
        $needAmpersand = false;

        $inv = new angelInvest();
        $invests = $inv->getInvestments($pay->companyID, $investorID);
        // angeldie("payoutBasis: " . serialize($invests)); // debug
        foreach ($invests as $i) {
            $id = $i->id;
            $key = "payoutID" . $id;
            if (array_key_exists($key, $_POST)) {
                if ($needAmpersand)
                    $basis .= "&$id=" . $_POST[$key];
                else {
                    $basis .= "$id=" . $_POST[$key];
                    $needAmpersand = true;
                }
            }
        }

        return($basis);

    }
}

/*
 * View for whoopie
 */

class whoopie_view extends angelview {


    function __construct() {
        parent::__construct();



        // $this->htmlHeader = "<title>Whoopie</title>";    // dunno why i'm doing this here - it's already done!

    }


    function createWhoopieDialogs() {


        $body = "";

        

        $year = $GLOBALS["currentYear"];
        $body .= "
                    <div id='dialogJoinGame' title='Join Whoopie Game' style='display:none'>
                            <form id='' action='' method='post' enctype='multipart/form-data'>
                            Please Choose Your Name to Join '<span id=whoopieJoinName></span>'

                             <p></p>
                                    <table style='width:90%'>
                                                   <tr> <td style='text-align: center'>
                                                            <select id='playerName' name='playerName' value=''>
                                                                <option value='' selected disabled>What's your name? </option>
                                                                <option value='Elizabeth'>  Elizabeth</option>
                                                                <option value='Steven'>     Steven</option>
                                                                <option value='Geoff'>      Geoff</option>
                                                                <option value='Jon'>        Jon</option>

                                                             </select>
                                                         </td>
                                                   </tr>


                                     </table>
                         </form>

                         </div>";
        
                         $body .= "
                         <div id='dialogMakeBid' title='Make a Bid' style='display:none'>
                                 <form id='' action='' method='post' enctype='multipart/form-data'>
                                 How many tricks you gonna take this hand?'</span>'
     
                                  <p></p>
                                         <table style='width:90%'>
                                                        <tr> <td style='text-align: center'>
                                                                 <select id='whoopieBid' name='whoopieBid'>
                                                                 <option value='' selected disabled>How many tricks will you take?</option>";
                                                                 
                                                            
                        $body .= "
                                                                  </select>
                                                              </td>
                                                        </tr>
     
     
                                          </table>
                              </form>
     
                              </div>";

     $body .= "
                        <div id='dialogYourDeal' title='Your Deal' style='display:none'>
                                
                                Deal <span id=whoopieDealerCardCount></span>, please!'

                                <p></p>
                            </div>";
     

        return($body);
    }

    /*
     * Return an assoc array of fund:model for funds for which the user has a model
     */
    function getMyFundsWithModels($conn, $id) {

        $funds = array();

        $mod = new angeldb();

        $mod->shareconn($conn);
        $q = "SELECT DISTINCT f.name,i.percentPerYear FROM fundtable as f LEFT JOIN fundmodeltable AS i ON f.id = i.fundID WHERE i.investorID=$id ORDER BY f.name ASC";
        $r = $mod->getRowsCustom($q);

        while ($row = mysqli_fetch_row($r)) {
            $funds[$row[0]] = $row[1];
        }
        // angeldie("getMyFundsWithModels: " . serialize($funds));
        return($funds);

    }

    function createLeftNav($investorID) {
        $leftNav = "";

        /*
         * Top Nav
         */
        $leftNav .= "<div id='angelcalcNavContainer' class='angelcalcNav' style='padding-top: 0px;'>";
        $leftNav .= "<table class='tl1 tr2' style='margin: 0px 0px 0px 7px; border:0px solid black;border-collapse:collapse; width:50%'>
                            <tr ><td id='topNavAngel' onclick='chooseNav(\"angel\")'> Angel </td><td></td>
                                <td style='border-left:0px solid black;padding: 5px;' id='topNavFunds' onclick='chooseNav(\"funds\")'> Funds </td></tr></table>";

        $leftNav .= "<div style='margin: 0px 0px 0px 0px' id='messageDiv'></div>";

        /*
         * Angel investments nav
         */
        $leftNav .= "<div id='angelcalcNavAngel' style='margin-top: 20px'>

                        <table class='tr2 tr3' style='margin: 0px 0px 0px 7px;width:98%'>
                            <tr><td colspan='2' id='investmentsNav'>     Investments</td><td id='addInvestmentNav'>+</td></tr>
                            <tr><td colspan='2' id='payoutsNav'>         Distributions</td><td id='addPayoutNav'>+</td></tr>
                            <tr><td colspan='2' id='companiesNav'>      Companies</td><td id='addCompanyNav'>+</td></tr>
                            <tr><td colspan='2' id='dealflowNav'>      Deal Flow</td><td id='addDealflowNav'>+</td></tr>
                            <tr><td colspan='2' id='valuesNav'>      Portfolio Value</td></tr>
                            <tr><td colspan='3'><hr></td></tr>
                            <tr><td colspan='2' id='trackerNav'>      Tracker</td><td id='addTrackerNav'>+</td></tr>
                            <tr><td colspan='3'><hr></td></tr>
                            <tr><td colspan='2' id='modelNav'>           Model Conversions</td></tr>
                            <tr><td colspan='2' id='compareNav'>         Compare Conversions</td></tr>
                            <tr><td colspan='3'><hr></td></tr>";
        $friends = angelGetFriends($investorID);
        if (sizeof($friends) != 0) {
            $leftNav .= "<tr><td colspan='2' id='friendNav'>      Friends</td><td id='addFriendNav'>+</td></tr>";
            $leftNav .= "<tr><td class='friendList' colspan='3'><div class='friendList' id='friendList' style='display: none'>";
            foreach ($friends as $f) {
                $name = angelTruncateString($f->firstname . " " . $f->lastname, 16);
                $leftNav .= "<a href='javascript:onclick=showFriendInvestments(\"$f->id\",\"$name\")'><span>$name</span></a><br/>";
            }
            $leftNav .= "<tr><td colspan='3'><hr></td></tr>";
            $leftNav .= "</div></td></tr>";
        }
        $advisors = angelGetAdvisors($investorID);
        if (sizeof($advisors) != 0) {
            $leftNav .= "<tr><td colspan='2' id='advisorNav'>      Advisors</td><td id='addAdvisorNav'>+</td></tr>";
            $leftNav .= "<tr><td class='friendList' colspan='3'><div class='friendList' id='advisorList' style='display: none'>";
            foreach ($advisors as $adv) {
                $name = angelTruncateString($adv->firstname . " " . $adv->lastname, 16);
                $leftNav .= "<a href='javascript:onclick=showAdvisor(\"$adv->id\",\"$name\")'><span>$name</span></a><br/>";
            }
            $leftNav .= "<tr><td colspan='3'><hr></td></tr>";
            $leftNav .= "</div></td></tr>";
        }


        $leftNav .= "
                            <tr><td colspan='2' id='importNav'>      Import  </td></tr>
                            <tr><td colspan='2' id='exportNav'>      Export</td></tr>
                            <tr><td colspan='2' id='taxNav'>      Tax</td></tr>
                            <tr><td colspan='3'><hr></td></tr>
                            <tr><td colspan='3' style='color:green'>    Filter</td></tr>
                            <tr><td></td><td onclick='AngelView.switchFilter(\"all\")' id='showAll'>              Show All</td></tr>
                            <tr><td></td><td onclick='AngelView.switchFilter(\"active\")' id='showActive'>           Show Active</td></tr>
                            <tr><td></td><td onclick='AngelView.switchFilter(\"acquired\")' id='showAcquired'>         Show Acquired</td></tr>
                            <tr><td></td><td onclick='AngelView.switchFilter(\"ipo\")' id='showIPO'>              Show IPOs</td></tr>
                            <tr><td></td><td onclick='AngelView.switchFilter(\"dead\")' id='showDead'>             Show Dead</td></tr>
                        </table>

                        <table class='tl1 tr2' style='width:90%; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px;'>
                            <tr><td colspan='2' style='color:green'>    Summary</td></tr>
                            <tr><td>            Companies:</td>  <td id='summaryCompanies'></td></tr>
                            <tr><td>           Investments:</td><td id='summaryInvestments'></td></tr>
                            <tr><td>            Invested:</td>   <td id='summaryInvested'></td></tr>
                            <tr><td>            Returned:</td>   <td id='summaryReturned'></td></tr>
                            <tr><td colspan='2'><hr></td></tr>
                            <tr><td>            Active Cos:</td>   <td id='summaryActive'></td></tr>
                            <tr><td>            Your Value:</td>   <td id='summaryValue'></td></tr>
                        </table>





        </div>";

        $leftNav .= "<div id='angelcalcNavFunds' style='display:none; margin-top: 20px;'>

                        <table class='tr2 tr3' style='margin: 0px 0px 0px 7px;width:98%'>
                            <tr><td colspan='2' id='fundsNav'>            Fund Commitments</td><td id='addFundInvestNav'>+</td></tr>
                            <tr><td colspan='2' id='fundPaymentsNav'>     Capital Payments</td><td id='addFundPaymentNav'>+</td></tr>
                            <tr><td colspan='2' id='fundPayoutsNav'>      Distributions</td><td id='addFundPayoutNav'>+</td></tr>

                            <tr><td colspan='2' id='fundPerformanceNav'>  Performance</td></tr>
                            <tr><td colspan='2' id='fundCashflowsNav'>    Cash Flows</td></tr>

                            <tr><td colspan='3'><hr></td></tr>";


        $leftNav .= "
                            <tr><td colspan='2' id='importFundsNav'>      Import  </td></tr>
                            <tr><td colspan='2' id='exportFundsNav'>      Export</td></tr>
                            <tr><td colspan='2' id='taxFundsNav'>         Tax</td></tr>
                            <tr><td colspan='3'><hr></td></tr>
                            <tr><td colspan='3' style='color:green'>    Filter</td></tr>
                            <tr><td></td><td onclick='FundsView.switchFilter(\"all\")' id='fundsShowAll'>              Show All</td></tr>
                            <tr><td></td><td onclick='FundsView.switchFilter(\"active\")' id='fundsShowActive'>           Show Active</td></tr>
                             <tr><td></td><td onclick='FundsView.switchFilter(\"carry\")' id='fundsShowCarry'>           Show Carry</td></tr>
                            <tr><td></td><td onclick='FundsView.switchFilter(\"closed\")' id='fundsShowClosed'>         Show Closed</td></tr>
                        </table>

                        <table class='tl1 tr2' style='width:90%; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px;'>
                            <tr><td colspan='2' style='color:green'>    Summary</td></tr>
                            <tr><td>            Funds:</td>  <td id='summaryFunds'></td></tr>
                            <tr><td>            Active Funds:</td>   <td id='summaryFundsActive'></td></tr>
                            <tr><td>            Committed:</td><td id='summaryFundsCommitted'></td></tr>
                            <tr><td>            Commit Left:</td>   <td id='summaryFundsCommitLeft'></td></tr>

                            <tr><td colspan='2'><hr></td></tr>

                            <tr><td>            Returned:</td>   <td id='summaryFundsReturned'></td></tr>
                        </table>

                        <div style='margin: 0px 0px 0px 0px' id='messageDiv'></div>



        </div>";        // angelcalcNavFunds



        $leftNav .= "</div>";        // angelcalcNavContainer
        /*
         *                        <div style='margin: 40px 2px 0px 7px;border: 0px solid green;height:300px;' id='historyDiv'>
                        <table id='historyTable' class='tl1 tr2' style='width:100%; border: 2px solid green;'>
                            <tr><td colspan='2' style='color:green'>    History</td></tr>
                            <tr><td style='font-size:10px'>9-25-16 (2:51pm)</td>   <td style='font-size:10px'>New invest in XYZ</td></tr>
                        </table>
                        </div>
         */
        return($leftNav);
    }

 /*
  * create a right side for the start page
  */
    function startPageRightSide($investorID, $p) {

        $currentYear = date("Y");
        $prevYear = $currentYear - 1;

        $page = "<div style='margin: 70px 0px 0px 300px; border: 0px solid black; width=600px'>";
        $page .= "<span style='color:green; font-size: 18px'>Value</span><p></p>";
        $page .= "
        <table class='tl1 tr2 tr3' style='width:500px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px; float:left'>
                            <tr><td></td><td style='color:green'>    Current Value</td><td style='color:green'>    Predicted Value</td></tr>
                            <tr><td>&nbsp</td></tr>
                            <tr><td>   Angel</td>   <td id='startNetworthAngelCurrent'></td><td id='startNetworthAngelPredicted'></td></tr>
                            <tr><td>   Funds</td>   <td id='startNetworthFundsCurrent'></td><td id='startNetworthFundsPredicted'></td></tr>
                            <tr><td>   Funds (Carry Only)</td>   <td id='startNetworthFundsCarryCurrent'></td><td id='startNetworthFundsCarryPredicted'></td></tr>
                            <tr><td colspan='3'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td>   Total</td>   <td id='startNetworthTotalCurrent'></td><td id='startNetworthTotalPredicted'></td></tr>
                        </table>
                        ";
        $page .= "</div>";

        $page .= "<div style='margin: 30px 0px 0px 850px; border: 0px solid black; width=400px;'>";
        $page .= "
        <table class='tl1 tr2' style='width:300px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px;'>
                            <tr><td>   Angel Investments </td>   <td id='startNoCompanies'></td></tr>
                            <tr><td>   Active Angel Investments </td>   <td id='startNoActiveCompanies'></td></tr>
                            <tr><td colspan='3'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td>   Funds </td>              <td id='startNoFunds'></td></tr>
                            <tr><td>   Funds with Commitments </td>   <td id='startNoFundsCommit'></td></tr>
                            <tr><td>    Funds With Carry </td>    <td id='startNoFundsCarry'></td></tr>
                            <tr><td>   Total Commit Left </td>   <td id='startCommitLeft'></td></tr>


                        </table>
                        ";
        $page .= "</div><br>";

        $page .= "<div style='margin: 25px 0px 0px 300px; border: 0px solid black; width=600px'>";
        $page .= "<span style='color:green; font-size: 18px'>Returns </span><p></p>

        <table class='tl1 tr2 tr3 tr4' style='width:850px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px;'>
                            <tr><td></td><td style='color:green'>   Actual</td><td style='color:green'>    Based on: Value</td> <td style='color:green'>    Based on: Prediction</td></tr>
                            <tr><td>Angel</td></tr>
                            <tr><td style='text-align: right'>Investment</td>   <td id='startActualAngelInvest'></td><td id='startValueAngelInvest'></td><td id='startPredictedAngelInvest'></td></tr>
                            <tr><td style='text-align: right'>Returns</td>   <td id='startActualAngelReturn'></td><td id='startValueAngelReturn'></td><td id='startPredictedAngelReturn'></td></tr>
                            <tr><td style='text-align: right'>ROI</td>   <td id='startActualAngelROI'></td><td id='startValueAngelROI'></td><td id='startPredictedAngelROI'></td></tr>
                            <tr><td colspan='4'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td>Funds</td>
                            <tr><td style='text-align: right'>Investment</td>   <td id='startActualFundsInvest'></td><td id='startValueFundsInvest'></td><td id='startPredictedFundsInvest'></td></tr>
                            <tr><td style='text-align: right'>Returns</td>   <td id='startActualFundsReturn'></td><td id='startValueFundsReturn'></td><td id='startPredictedFundsReturn'></td></tr>
                            <tr><td style='text-align: right'>ROI</td>   <td id='startActualFundsROI'></td><td id='startValueFundsROI'></td><td id='startPredictedFundsROI'></td></tr>

                            <tr><td colspan='4'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td>   Total</td>
                            <tr><td style='text-align: right'>Investment</td>   <td id='startActualTotalInvest'></td><td id='startValueTotalInvest'></td><td id='startPredictedTotalInvest'></td></tr>
                            <tr><td style='text-align: right'>Returns</td>   <td id='startActualTotalReturn'></td><td id='startValueTotalReturn'></td><td id='startPredictedTotalReturn'></td></tr>
                            <tr><td style='text-align: right'>ROI</td>   <td id='startActualTotalROI'></td><td id='startValueTotalROI'></td><td id='startPredictedTotalROI'></td></tr>
                        </table>
                        ";
        $page .= "</div>";


        /*
         * Activity over time
         */
        $page .= "<div style='margin: 25px 0px 0px 300px; border: 0px solid black; width=1200px'>";

        $page .= "<span style='color:green; font-size: 18px'>Activity Over Time ($ in '000s) </span><p></p>

        <table class='tr1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12' style='width:1000px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px; float:left'>
            <tr><td></td>";
        for ($year=$currentYear - 9; $year <= $currentYear; $year++ ) {
            $page .= "<td style='color:green'>  $year</td>";
        }
        $page .= "<td style='color:green'>  Totals</td></tr>

                            <tr><td style='text-align: left'>Angel</td></tr>
                            <tr><td style='text-align: right'>Investments</td><td id='invYM9'></td><td id='invYM8'></td><td id='invYM7'></td><td id='invYM6'></td>
                                                                              <td id='invYM5'></td><td id='invYM4'></td><td id='invYM3'></td><td id='invYM2'></td>
                                                                              <td id='invYM1'></td><td id='invYM0'></td><td style='color:blue' id='invBYTotal'></td>

                            </tr>

                            <tr><td style='text-align: right'>Amount</td>     <td id='amtYM9'></td><td id='amtYM8'></td><td id='amtYM7'></td><td id='amtYM6'></td>
                                                                              <td id='amtYM5'></td><td id='amtYM4'></td><td id='amtYM3'></td><td id='amtYM2'></td>
                                                                              <td id='amtYM1'></td><td id='amtYM0'></td><td style='color:blue'  id='amtBYTotal'></td>

                            </tr>
                            <tr><td style='text-align: right'>Distributions</td> <td id='adistYM9'></td><td id='adistYM8'></td><td id='adistYM7'></td><td id='adistYM6'></td>
                                                                              <td id='adistYM5'></td><td id='adistYM4'></td><td id='adistYM3'></td><td id='adistYM2'></td>
                                                                              <td id='adistYM1'></td><td id='adistYM0'></td><td style='color:blue'  id='adistBYTotal'></td>

                            </tr>

                            <tr><td colspan='12'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td  style='text-align: left'>Funds</td>
                            <tr><td style='text-align: right'>Capital Calls</td><td id='ccYM9'></td><td id='ccYM8'></td><td id='ccYM7'></td><td id='ccYM6'></td>
                                                                              <td id='ccYM5'></td><td id='ccYM4'></td><td id='ccYM3'></td><td id='ccYM2'></td>
                                                                              <td id='ccYM1'></td><td id='ccYM0'></td><td style='color:blue'  id='ccBYTotal'></td>
                            </tr>
                            <tr><td style='text-align: right'>Dists Invest</td><td id='fdistYM9'></td><td id='fdistYM8'></td><td id='fdistYM7'></td><td id='fdistYM6'></td>
                                                                              <td id='fdistYM5'></td><td id='fdistYM4'></td><td id='fdistYM3'></td><td id='fdistYM2'></td>
                                                                              <td id='fdistYM1'></td><td id='fdistYM0'></td><td style='color:blue'  id='fdistBYTotal'></td>
                            </tr>
                            <tr><td style='text-align: right'>Dists Carry</td><td id='fdistcYM9'></td><td id='fdistcYM8'></td><td id='fdistcYM7'></td><td id='fdistcYM6'></td>
                                                                              <td id='fdistcYM5'></td><td id='fdistcYM4'></td><td id='fdistcYM3'></td><td id='fdistcYM2'></td>
                                                                              <td id='fdistcYM1'></td><td id='fdistcYM0'></td><td style='color:blue'  id='fdistcBYTotal'></td>
                            </tr>

                            <tr><td colspan='12'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td  style='text-align: left'>Totals</td>
                            <tr><td style='text-align: right'>Outflows</td>   <td id='ofYM9'></td><td id='ofYM8'></td><td id='ofYM7'></td><td id='ofYM6'></td>
                                                                              <td id='ofYM5'></td><td id='ofYM4'></td><td id='ofYM3'></td><td id='ofYM2'></td>
                                                                              <td id='ofYM1'></td><td id='ofYM0'></td><td style='color:blue'  id='ofBYTotal'></td>
                            </tr>
                            <tr><td style='text-align: right'>Inflows</td>    <td id='ifYM9'></td><td id='ifYM8'></td><td id='ifYM7'></td><td id='ifYM6'></td>
                                                                              <td id='ifYM5'></td><td id='ifYM4'></td><td id='ifYM3'></td><td id='ifYM2'></td>
                                                                              <td id='ifYM1'></td><td id='ifYM0'></td><td style='color:blue'  id='ifBYTotal'></td>
                            </tr>
                             <tr><td colspan='12'><div style='width:100%; border: 1px dotted blue'></div></td></tr>
                            <tr><td style='text-align: right'>Net</td>        <td id='netYM9'></td><td id='netYM8'></td><td id='netYM7'></td><td id='netYM6'></td>
                                                                              <td id='netYM5'></td><td id='netYM4'></td><td id='netYM3'></td><td id='netYM2'></td>
                                                                              <td id='netYM1'></td><td id='netYM0'></td><td style='color:blue'  id='netBYTotal'></td>
                            </tr>


                        </table>
                        ";
        $page .= "</div>";      // activity over time


        $page .= "<div style='margin: 30px 0px 0px 300px; border: 0px solid black; width:1000px;'>";
        $page .= "<span style='color:green; font-size: 18px'>Activity </span><p></p>";
        $page .= "<div style='margin: 0px 0px 0px 0px; border: 0px solid black; width:600px; float:left'>";

        $page .= "<table class='tl1 tl2 tl3' style='width:550px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px; font-size: 10px'>
                            <tr><th style='font-size:14px;width:80px'> Company  </th><th style='font-size:14px;width:100px'> Publish Date  </th><th style='font-size:14px'>Source</th><th style='font-size:14px'>Story</th></tr>
                            <tbody>
                            <tr><td colspan='2' style='text-align:center; color:red'> &nbsp  </td></tr>";
        $n = new angelNews();
        $n->createMap($companyMap, "companytable", "id", "name");

        // $n->updateSavedNews($investorID);    // this was for debugging, should be called from cron job daily
        $newsResults = $n->getAllNewsfromDB($investorID);

        //angeldie("xxx: " . serialize($news));

        while($n = mysqli_fetch_object($newsResults,"angelNews")){

            $source = $n->source;
            $title = $n->title;
            $url = $n->url;
            $coName = $companyMap[$n->companyID];
            $newsDate = substr($n->newsDate, 0, 10);

            $weekAgoTime = strtotime("-1 week");
            $newsTime = strtotime($newsDate);

            if ($source == "" or $title == "" or $url == "" or $newsTime < $weekAgoTime)
                continue;        // skip empty stories

            $page .=   "<tr><td>" . $coName . "</td><td> " . $newsDate . "</td><td>" . $source . "</td><td>" . "<a href='$url'>" . $title . "</a></td></tr>";
        }
        $page .= "</tbody></table>";

        $page .= "</div>";


        $page .= "<div style='margin: 30px 0px 0px 450px; border: 0px solid black; width=500px;'>";
        $page .= "<table class='tl1 tl2 tl3' style='width:400px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px; font-size: 10px'>
                            <tr><th style='font-size:14px'>  Date </th><th style='font-size:13px'>Activity</th><th style='font-size:14px'>Details</th></tr>
                            <tbody>
                            <tr><td colspan='3' style='text-align:center; color:red'> &nbsp </td></tr>";
        $newEntries = $this->getNewEntries(15);
        foreach ($newEntries as $ent) {
            $page .= $ent;
        }
        $page .= "</tbody></table>";

        $page .= "</div></div><br><p></p>";





        return($page);

    }   // startPageRightSide()

    /*
  * create a right side for the start page
  */
    function cashflowPageRightSide($investorID, $p) {

        $currentYear = date("Y");
        $prevYear = $currentYear - 1;

        $page = "";

        /*
         * Activity over time
         */
        $page .= "<div style='margin: 25px 0px 0px 300px; border: 0px solid black; width=1200px'>";

        $page .= "<span style='color:green; font-size: 18px'>Cash Flows Over Time ($ in '000s) </span><p></p>

        <table class='tr1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12' style='width:1020px; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px; float:left'>
            <tr><td></td><td colspan='10'>
                   <table style='margin: 0px 0px 0px 40px; width:800px'>
                   <tr><td><input onchange='updateCashflowPage();' type='range' min='1' max='10' value='5' class='cflowSlider' id='cflowRange'></td></tr>
                   </table>
                   </td></tr>
            <tr><td></td>";
        $yearCount = 0;
        for ($year=$currentYear - 9; $year <= $currentYear; $year++ ) {
            $id = "cashFlow" . $yearCount;
            $yearCount++;
            $page .= "<td style='color:green; width:100px' id='$id'>  $year</td>";
        }
        $page .= "<td style='color:green; width:120px'>  Totals</td></tr>


                            <tr><td colspan='12'><div style='width:100%; border: 1px solid green'></div></td></tr>
                            <tr><td style='text-align: left'>Income</td></tr>
                            <tr><td style='text-align: right'>Salary</td></tr>
                            <tr><td style='text-align: right'></td><td id='salYM9'></td><td id='salYM8'></td><td id='salYM7'></td><td id='salYM6'></td>
                                                                              <td id='salYM5'></td><td id='salYM4'></td><td id='salYM3'></td><td id='salYM2'></td>
                                                                              <td id='salYM1'></td><td id='salYM0'></td><td style='color:blue' id='salBYTotal'></td>

                            </tr>
                            <tr><td style='text-align: right'>Interest and Dividends</td></tr>
                            <tr><td style='text-align: right'></td><td id='iadYM9'></td><td id='iadYM8'></td><td id='iadYM7'></td><td id='iadYM6'></td>
                                                                              <td id='iadYM5'></td><td id='iadYM4'></td><td id='iadYM3'></td><td id='iadYM2'></td>
                                                                              <td id='iadYM1'></td><td id='iadYM0'></td><td style='color:blue' id='iadBYTotal'></td>

                            </tr>
                            <tr><td colspan='12'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td style='text-align: left'>Expenses</td></tr>




                            <tr><td colspan='12'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td style='text-align: left'>Angel</td></tr>
                            <tr><td style='text-align: right'>Investments</td><td id='invYM9'></td><td id='invYM8'></td><td id='invYM7'></td><td id='invYM6'></td>
                                                                              <td id='invYM5'></td><td id='invYM4'></td><td id='invYM3'></td><td id='invYM2'></td>
                                                                              <td id='invYM1'></td><td id='invYM0'></td><td style='color:blue' id='invBYTotal'></td>

                            </tr>

                            <tr><td style='text-align: right'>Amount</td>     <td id='amtYM9'></td><td id='amtYM8'></td><td id='amtYM7'></td><td id='amtYM6'></td>
                                                                              <td id='amtYM5'></td><td id='amtYM4'></td><td id='amtYM3'></td><td id='amtYM2'></td>
                                                                              <td id='amtYM1'></td><td id='amtYM0'></td><td style='color:blue'  id='amtBYTotal'></td>

                            </tr>
                            <tr><td style='text-align: right'>Distributions</td> <td id='adistYM9'></td><td id='adistYM8'></td><td id='adistYM7'></td><td id='adistYM6'></td>
                                                                              <td id='adistYM5'></td><td id='adistYM4'></td><td id='adistYM3'></td><td id='adistYM2'></td>
                                                                              <td id='adistYM1'></td><td id='adistYM0'></td><td style='color:blue'  id='adistBYTotal'></td>

                            </tr>

                            <tr><td colspan='12'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td  style='text-align: left'>Funds</td>
                            <tr><td style='text-align: right'>Capital Calls</td><td id='ccYM9'></td><td id='ccYM8'></td><td id='ccYM7'></td><td id='ccYM6'></td>
                                                                              <td id='ccYM5'></td><td id='ccYM4'></td><td id='ccYM3'></td><td id='ccYM2'></td>
                                                                              <td id='ccYM1'></td><td id='ccYM0'></td><td style='color:blue'  id='ccBYTotal'></td>
                            </tr>
                            <tr><td style='text-align: right'>Dists Invest</td><td id='fdistYM9'></td><td id='fdistYM8'></td><td id='fdistYM7'></td><td id='fdistYM6'></td>
                                                                              <td id='fdistYM5'></td><td id='fdistYM4'></td><td id='fdistYM3'></td><td id='fdistYM2'></td>
                                                                              <td id='fdistYM1'></td><td id='fdistYM0'></td><td style='color:blue'  id='fdistBYTotal'></td>
                            </tr>
                            <tr><td style='text-align: right'>Dists Carry</td><td id='fdistcYM9'></td><td id='fdistcYM8'></td><td id='fdistcYM7'></td><td id='fdistcYM6'></td>
                                                                              <td id='fdistcYM5'></td><td id='fdistcYM4'></td><td id='fdistcYM3'></td><td id='fdistcYM2'></td>
                                                                              <td id='fdistcYM1'></td><td id='fdistcYM0'></td><td style='color:blue'  id='fdistcBYTotal'></td>
                            </tr>

                            <tr><td colspan='12'><div style='width:100%; border: 2px solid blue'></div></td></tr>
                            <tr><td  style='text-align: left'>Totals</td>
                            <tr><td style='text-align: right'>Outflows</td>   <td id='ofYM9'></td><td id='ofYM8'></td><td id='ofYM7'></td><td id='ofYM6'></td>
                                                                              <td id='ofYM5'></td><td id='ofYM4'></td><td id='ofYM3'></td><td id='ofYM2'></td>
                                                                              <td id='ofYM1'></td><td id='ofYM0'></td><td style='color:blue'  id='ofBYTotal'></td>
                            </tr>
                            <tr><td style='text-align: right'>Inflows</td>    <td id='ifYM9'></td><td id='ifYM8'></td><td id='ifYM7'></td><td id='ifYM6'></td>
                                                                              <td id='ifYM5'></td><td id='ifYM4'></td><td id='ifYM3'></td><td id='ifYM2'></td>
                                                                              <td id='ifYM1'></td><td id='ifYM0'></td><td style='color:blue'  id='ifBYTotal'></td>
                            </tr>
                             <tr><td colspan='12'><div style='width:100%; border: 1px dotted blue'></div></td></tr>
                            <tr><td style='text-align: right'>Net</td>        <td id='netYM9'></td><td id='netYM8'></td><td id='netYM7'></td><td id='netYM6'></td>
                                                                              <td id='netYM5'></td><td id='netYM4'></td><td id='netYM3'></td><td id='netYM2'></td>
                                                                              <td id='netYM1'></td><td id='netYM0'></td><td style='color:blue'  id='netBYTotal'></td>
                            </tr>


                        </table>
                        ";
        $page .= "</div>";      // activity over time


        return($page);

    }   // cashflowPageRightSide
 /*
  * create a left side for the start page (user profile plus maybe history)
  */
    function startPageLeftSide($investorID, $p) {

        if ($p->photo == NULL or $p->photo == "")
            $photo = "/img/founder.jpeg";
        else
            $photo     = $p->photo;

        $co = new angelCompany();
        $co->connect();      // use the existing connection

        $co->id = $p->company_id;

        if ($co->id != 0)
            $co = $co->findMatchingCompany();
        else
            $co = NULL;

        $page = "<div class='companyPageNavDiv'>";

        $page .= "<div style='margin: 0px 0px 15px 0px; font-size: 20px; border: 0px solid green;clear:both;'><a href='javascript:onclick=toggleStartPageAndTables()' class='modelButton'>Manage Investments</a></div><br />";


        if ($p->email == "") {      // This is a new founder
            $page .= "<span style='font-size:16px; font-weight: bold '> New Founder! </span><br />";
        } else {
            $page .= "<span style='font-size:16px; font-weight: bold '> $p->firstname $p->lastname </span><br />";

        }
        if ($p->company_id and $co != NULL)
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
        $page .= "<div id='startPageCompanySummary' class='angelbasic angelGraphics' style='margin: 50px 0px 0px 3px;width: 220px;height: 220px'></div>";

        $page .= "<p></p>";
        $page .= "<div style='margin: 30px 0px 0px 0px; font-size: 20px; border: 0px solid green;float:left; clear:both;'><a href='javascript:onclick=toggleStartPageAndTables()' class='modelButton'>Manage Investments</a></div><br />";

        $page .= "</div>";

        return($page);

    }   // startPageLeftsideSummary

    /*******************************************************************************************
     *      Body for main funds page. Most of the work is done in jscript (angel.js)
     *
     * @param $investorID
     * @return string
     */

    function fundsContainers() {
        $body = "";

        $body .= "<div id='fundsContainer' style='display:none'>";
        $body .= "<div id='transparentFundsHelpDiv' style='opacity:1; color:blue; z-index:99; display:none; width: 250px; position:absolute; top:100px; height:250px; border: 3px dotted blue; margin: 0px 0px 0px 220px; float:left; padding: 6px'>";
        $body .= "<-- The easiest way to begin is to click on this '+' sign to add a new investment.<p /><p />
                    Alternatively, to add a number of investments (or distributions) at once, click on 'Import' here:
                  <p />

                    <-- Import

                ";

        $body .= "</div>";  // transparentFundsHelpDiv


        /*
         * Fund committments / investments table
         */
        $body .= "<div id='fundsTableHeaderDiv' class='angelbasic angelTableHeader' style='border: 0px solid orange; width:1090px;'>
                <table class='angelFundsTable' style='border: 0px solid black;'>
                <thead>
                <tr>
                    <th id='angelFundsHeaderCol1' mySort='fund'    myProp='id' class='header' valign=top style='width:270px'>Fund</th>
                    <th id='angelFundsHeaderCol2' mySort='date'    myProp='invDate'class='header' valign=top style='width:100px'>Date</th>
                    <th id='angelFundsHeaderCol3' mySort='number'  myProp='commit' class='header' valign=top style='width:110px'>Commitment</th>
                    <th id='angelFundsHeaderCol4' mySort='number'  myProp='commitPaid' class='header' valign=top style='width:110px'>Called</th>
                    <th id='angelFundsHeaderCol5' mySort='number'  myProp='leftOfCommit' class='header' valign=top style='width:100px'>Left to Call</th>
                    <th id='angelFundsHeaderCol6' mySort='number'  myProp='returned' class='header' valign=top style='width:100px'>Returned</th>
                    <th id='angelFundsHeaderCol7' mySort='number'  myProp='ROI' class='header' valign=top style='width:75px'>ROI</th>
                    <th id='angelFundsHeaderCol8' mySort='string'  myProp='notes' class='header' valign=top style='width:200px'>Notes</th>
                    <th class='header' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"funds\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                 </tr></thead></table></div>";

        $body .= "<div id='fundsTableDiv' class='angelbasic angelTableBody' style='width:1160px;'>";
        $body .= "<table id='fundsTable'  class='angelFundsTable tl1 tc2 tr3 tr4 tr5 tr6 tr7 tl8'>";
        $body .= "  <thead>
                    <tr>
                        <th valign=top style='width:270px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:110px'></th>
                        <th valign=top style='width:110px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:75px'></th>
                        <th valign=top style='width:200px'></th>
                    </tr></thead><tbody style='height:1200px'></tbody></table></div>";

        $body .= "</div>";  // fundsContainer

        /*
         * Fund Payouts and payments tables
         */
        $body .= "<div id='fundPayoutsContainer' style='display:none'>";
        $body .= "<div id='fundPayoutsTableHeaderDiv' class='angelbasic angelTableHeader' width: 1100px;'>";
        $body .= "<table class='angelPayoutsTable'>
                    <thead>
                    <tr>
                        <th id='angelFundPayoutsHeaderCol1' mySort='fund'    myProp='fundID' class='header' valign=top style='width:200px'>Fund</th>
                        <th id='angelFundPayoutsHeaderCol2' mySort='date'    myProp='payDate'class='header' valign=top style='width:100px'>Date</th>
                        <th id='angelFundPayoutsHeaderCol3' mySort='string'  myProp='type' class='header' valign=top style='width:100px'>Type</th>
                        <th id='angelFundPayoutsHeaderCol4' mySort='number'  myProp='cash' class='header' valign=top style='width:100px'>Cash</th>
                        <th id='angelFundPayoutsHeaderCol5' mySort='number'  myProp='shares' class='header' valign=top style='width:100px'>Shares</th>
                        <th id='angelFundPayoutsHeaderCol6' mySort='string'  myProp='symbol' class='header' valign=top style='width:100px'>Symbol</th>
                        <th id='angelFundPayoutsHeaderCol7' mySort='number'  myProp='sharePrice' class='header' valign=top style='width:100px'>Price</th>
                        <th id='angelFundPayoutsHeaderCol8' mySort='number'  myProp='value' class='header' valign=top style='width:100px'>Value</th>
                        <th id='angelFundPayoutsHeaderCol9' mySort='string'  myProp='notes' class='header' valign=top style='width:180px'>Notes</th>
                        <th class='header' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"fundPayouts\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                     </tr></thead></table></div>";

        $body .= "<div id='fundPayoutsTableDiv' class='angelbasic angelTableBody' style='width:1190px;'>";
        $body .= "<table id='fundPayoutsTable' class='angelPayoutsTable tl1 tr3 tr4 tr5 tr6 tr7 tr8 tl9'>";
        $body .= "  <thead>
                    <tr>
                        <td valign=top style='width:200px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:180px;'></td>
                    </tr></thead><tbody></tbody></table></div>";

        $body .= "</div>";  // fundPayoutsContainer

        $body .= "<div id='fundPaymentsContainer' style='display:none'>";
        $body .= "<div id='fundPaymentsTableHeaderDiv' class='angelbasic angelTableHeader' style='width: 900px;'>";
        $body .= "<table class='angelPaymentsTable' style='border: 0px solid black;'>
                    <thead>
                    <tr>
                        <th id='angelFundPaymentsHeaderCol1' mySort='fund'    myProp='fundID' class='header' valign=top style='width:250px'>Fund</th>
                        <th id='angelFundPaymentsHeaderCol2' mySort='date'    myProp='invDate' class='header' valign=top style='width:125px'>Date</th>
                        <th id='angelFundPaymentsHeaderCol3' mySort='number'  myProp='investAmount' class='header' valign=top style='width:125px'>Amount</th>
                        <th id='angelFundPaymentsHeaderCol4' mySort='special'  myProp='leftOfCommit' class='header' valign=top style='width:125px'>Left to Call</th>
                        <th id='angelFundPaymentsHeaderCol5' mySort='string'  myProp='notes' class='header' valign=top style='width:275px'>Notes</th>
                        <th class='' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"fundPayments\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                     </tr></thead></table></div>";

        $body .= "<div id='fundPaymentsTableDiv' class='angelbasic angelTableBody' style='width:970px;'>";
        $body .= "<table id='fundPaymentsTable' class='angelPaymentsTable tl1 tc2 tr3 tr4 tl5'>";
        $body .= "  <thead>
                    <tr>
                        <td valign=top style='width:250px'></td>
                        <td valign=top style='width:125px'></td>
                        <td valign=top style='width:125px'></td>
                        <td valign=top style='width:125px'></td>
                        <td valign=top style='width:275px;margin:20px'></td>
                    </tr></thead><tbody></tbody></table></div>";

        $body .= "</div>";  // fundPaymentsContainer

        /*
         * Performance
         */
        $body .= "<div id='fundPerformanceContainer' style='display:none'>";
        $body .= "<div id='fundPerformanceTableHeaderDiv' class='angelbasic angelTableHeader' style='width: 1200px;'>";
        $fillerColor = "aliceblue";
        $body .= "<table class='angelValuesTable'>
                    <thead>
                    <tr>
                        <th id='angelFundPerformanceHeaderCol1' mySort='fund'    myProp='id' class='header' valign=top style='width:200px;'>Fund</th>
                        <th id='angelFundPerformanceHeaderCol2' mySort='special'    myProp='invested' class='header' valign=top style='width:100px'>Invested</th>
                        <th id='angelFundPerformanceHeaderCol3' mySort='special'  myProp='returned' class='header' valign=top style='width:100px'>Returned</th>
                        <th id='angelFundPerformanceHeaderCol4' mySort='special'  myProp='capitalAccount' class='header' valign=top style='width:100px'>Cap Acct</th>
                        <th id='angelFundPerformanceHeaderCol5' mySort='special'  myProp='predictedReturn' class='header' valign=top style='width:100px;
                                                                                            margin-right:2px;'>Prediction</th>

                        <th style='background-color:$fillerColor;width:5px;'>&nbsp</th>

                        <th id='angelFundPerformanceHeaderCol6' mySort='special'  myProp='actualIRR' class='header' valign=top style='width:85px'>Current <br/>IRR</th>
                        <th id='angelFundPerformanceHeaderCol7' mySort='special'  myProp='actualTVPI' class='header' valign=top style='width:85px;border-right:'>Current<br/> TVPI</th>
                        <th style='background-color:$fillerColor;width:5px;'>&nbsp</th>
                        <th id='angelFundPerformanceHeaderCol8' mySort='special'  myProp='TVPIplusCA' class='header' valign=top style='width:85px;'>TVPI incl<br/> Cap Acct</th>
                        <th style='background-color:$fillerColor;width:5px;'>&nbsp</th>
                        <th id='angelFundPerformanceHeaderCol9' mySort='special'  myProp='TVPIplusPredict' class='header' valign=top style='width:92px'>TVPI incl <br/>Predict</th>
                        <th class='' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"fundPerformance\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                     </tr></thead></table></div>";

        $body .= "<div id='fundPerformanceTableDiv' class='angelbasic angelTableBody' style='width:1100px;'>";
        $body .= "<table id='fundPerformanceTable' class='angelValuesTable tl1 tr2 tr3 tr4 tr5'>";
        $body .= "  <thead>
                    <tr>
                        <th valign=top style='width:200px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px; margin-right:2px'></th>
                        <th style='background-color:$fillerColor;width:5px'></th>
                        <th valign=top style='width:85px;'></th>
                        <th valign=top style='width:85px;'></th>
                        <th style='background-color:$fillerColor;width:5px'></th>
                        <th valign=top style='width:85px;'></th>
                        <th style='background-color:$fillerColor;width:5px'></th>
                        <th valign=top style='width:92px;'></th>
                    </tr></thead><tbody></tbody></table></div>";

        $body .= " <div class='angelbasic angelTableBody' style='clear:both; width:1180px; margin-top: 0px'>";
        $body .= " <table id='fundPerformanceTotalsTable' class='angelFundsTable  tl1 tr2 tr3 tr4 tr5 tc6 tc7 tc8 tc9'>
                         <thead>
                         <tr>
                        <th valign=top style='width:200px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px; margin-right:2px'></th>
                        <th style='background-color:$fillerColor;width:5px'></th>
                        <th valign=top style='width:85px;'></th>
                        <th valign=top style='width:85px;'></th>
                        <th style='background-color:$fillerColor;width:5px'></th>
                        <th valign=top style='width:85px;'></th>
                        <th style='background-color:$fillerColor;width:5px'></th>
                        <th valign=top style='width:92px;'></th>";

        $body .= "            </tr></thead><tbody></tbody></table>";
        $body .= "     </div>";

        $body .= "</div>";  // fundPerformanceContainer
        /*
         * cash flow
         */

        $currentYear = date("Y");

        $body .= "<div id='fundCashflowsContainer' style='display:none;'>";
        /*
         * Cashflow chooser (actuals or predictions
         */
        $body .= "   <div style='margin:70px 0px 0px 240px '>
                                  <a href='#' id='actualCashFlowsChoice' style='color:black'
                                     onclick='cashflowChooser(this,\"actuals\")'>Actual Cash Flows</a> | <a href='#' id='predictedCashFlowsChoice'
                                     onclick='cashflowChooser(this,\"predictions\")' >Predicted Cash Flows</a> &nbsp (all numbers in thousands) &nbsp";

        $body .= "<span id='includeCapitalPaymentsSpan' style='font-weight: 100; font-size: 12px; margin-left: 50px'>
                                <input onchange='showFundCashflows(0)' type='checkbox' id='includePayments' name='includePayments' value='Y'>
                                                                          Include Capital Payments</span>";
        $body .= "</div><br/>";

        $body .= "   <div id='fundActualsContainer' class='angelbasic' style=''>";

        $body .= "     <div id='fundActualsTableHeaderDiv' class='angelbasic angelTableHeader' style='margin-top:0px; width:1150px '>";
        $body .= "      <table class='fundCashflowTable  tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12 tr13'>
                                  <thead>
                                     <tr>";
        $body .= "                   <th id='' class='' valign=top style='width:200px'>Fund</th>";
        $body .= "                   <th id='' class='' valign=top style='width:70px'>Previous</th>";

        for ($date = $currentYear-10; $date <= $currentYear; $date++) {
            $body .= "               <th id='' class='' valign=top style='width:70px'>$date</th>";
        }
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'>Totals</th>";
        $body .= "                   <th class='' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"fundActuals\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>";
        $body .= "            </tr></thead></table></div>";

        $body .= "     <div id='fundActualsTableDiv' class='angelbasic angelTableBody' style='width:1190px'>";
        $body .= "      <table id='fundActualsTable'class='fundCashflowTable  tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12 tr13'>
                                  <thead>
                                     <tr>";
        $body .= "                   <th id='' class='' valign=top style='width:200px'></th>";
        $body .= "                   <th id='' class='' valign=top style='width:70px'></th>";

        for ($date = $currentYear-10; $date <= $currentYear; $date++) {
            $body .= "               <th id='' class='' valign=top style='width:70px'></th>";
        }
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'></th>";
        $body .= "            </tr></thead><tbody></tbody></table>";

        $body .= "     </div>";
        $body .= "    <div id='fundActualsTotalsDiv' class='angelbasic angelTableBody' style='clear:both; width:1200px;'>";
        $body .= "         <table id='fundActualsTotalsTable' class='fundCashflowTable  tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12 tr13'>
                                  <thead>
                                     <tr>";
        $body .= "                   <th id='' class='' valign=top style='width:200px'></th>";
        $body .= "                   <th id='' class='' valign=top style='width:70px'></th>";
        for ($date = $currentYear-10; $date <= $currentYear; $date++) {
            $body .= "               <th id='' class='' valign=top style='width:70px'></th>";
        }
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'></th>";
        $body .= "               </tr></thead><tbody></tbody></table>";
        $body .= "     </div>";

        $body .= "   </div>";  // fundActualsContainer

        /*
         * Predictions!
         */
        $lastYear = $currentYear + 9;
        $body .= "   <div id='fundPredictionsContainer' class='angelbasic style='display: none'>";
        $body .= "     <div  id='fundPredictionsTableHeaderDiv' class='angelbasic angelTableHeader' style='margin-top:0px; width:1000px '>";
        $body .= "      <table class='fundCashflowTable tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12 tr13' style='border: 0px solid black;'>
                             <thead>
                               <tr>";
        $body .= "                   <th id='' class='' valign=top style='width:200px'>Fund</th>";
        $body .= "                   <th id='' class='' valign=top style='width:80px; color:blue'>Current</th>";
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'>Predicted</th>";
        for ($date = $currentYear; $date <= $lastYear; $date++) {
            $body .= "               <th id='' class='' valign=top style='width:70px'>$date</th>";
        }

        $body .= "                   <th class='' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"fundPredictions\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>";
        $body .= "            </tr></thead></table></div>";

        $body .= "     <div id='fundPredictionsTableDiv' class='angelbasic angelTableBody' style='width:1080px '>";
        $body .= "      <table id='fundPredictionsTable'class='fundCashflowTable  tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12 tr13'>
                              <thead>
                               <tr>";
        $body .= "                   <th id='' class='' valign=top style='width:200px'></th>";
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'></th>";
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'></th>";
        for ($date = $currentYear; $date <= $lastYear; $date++) {
            $body .= "               <th id='' class='' valign=top style='width:70px'></th>";
        }
        $body .= "            </tr></thead><tbody></tbody></table>";

        $body .= "     </div>";
        $body .= "    <div id='fundPredictionsTotalsDiv' class='angelbasic angelTableBody' style='clear:both; width:1100px;'>";
        $body .= "      <table id='fundPredictionsTotalsTable' class='fundCashflowTable tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8 tr9 tr10 tr11 tr12 tr13'>
                                  <thead>
                                     <tr>";
        $body .= "                   <th id='' class='' valign=top style='width:200px'></th>";
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'></th>";
        $body .= "                   <th id='' class='' valign=top style='width:80px;color:blue'></th>";

        for ($date = $currentYear; $date <= $lastYear; $date++) {
            $body .= "               <th id='' class='' valign=top style='width:70px'></th>";
        }
        $body .= "            </tr></thead><tbody></tbody></table>";
        $body .= "     </div>";


        $body .= "   </div>";  // fundPredictionsContainer

        $body .= "</div>";  // fundCashflowContainer


        return($body);
    }

    function fundPaymentEmailPageBody($investorID) {


        $fund = new angelFund();
        $conn = $fund->connect();

        $amount = number_format($_POST['paymentAmount']);
        $fund->id = $_POST['fund'];
        $foundFund = $fund->findMatchingFund(true);

        if ($foundFund == NULL) {
            angeldie("fundPaymentEmail called w/illegal fund ID: " . $fund->id);
        }
        $inv = new angelPerson();
        $conn = $inv->connect();
        $inv->id = $investorID;
        $inv =  $inv->findMatchingPerson(NULL);
        if ($inv == NULL) {
            angeldie("fundPaymentEmail called w/illegal investor ID: " . $investorID);
        }

        if ($foundFund->wireAcctName == null)
            $name = $fund->name;
        else
            $name = $foundFund->wireAcctName;

        $date = $_POST['date'];

        $emailText = "Dear Alisa,<br>
                 Please wire \$$amount to $name on $date from my private equity account (*1608) as described below and, as always, confirm via my cell. Thanks!<p></p>
                 Best,<br> $inv->firstname
";
        $body = "<script>$(document).ready( function() {\$.notify('Fund Payment Added', 'success');} );</script>";
        $body .= "<div style='display:block'>";

        /*
         *  Notify of errors and messages and add company summary on the left
         */
        if ($this->error != "")
            $body .= "<script>$(document).ready( function() {\$.notify('$this->error', 'error');} );</script>";

        $body .= "<div style='margin: 100px 0px 0px 240px'>";


        $body .= $emailText;
        $body .= "<p></p><p></p><table style='border: 1px solid blue; border-spacing: 10px'>";

        $body .= "<tr><td>Bank:</td><td>$foundFund->wireBank</td></tr>";
        $body .= "<tr><td>Bank Address:</td><td>$foundFund->wireBankAddress</td></tr>";
        $body .= "<tr><td>ABA Routing Number:</td><td>$foundFund->wireBankRouting</td></tr>";
        $body .= "<tr><td>Bank Account Number:</td><td>$foundFund->wireBankAcct</td></tr>";
        $body .= "<tr><td>Bank Account Name:</td><td>$foundFund->wireAcctName</td></tr>";
        $body .= "<tr><td>Reference:</td><td>$foundFund->wireReference</td></tr>";

        $body .= "</table>";

        $body .= "<p></p><a href='/investments?display=investments'>Return to investments</a>";

        $body .= "</div></div>";

        return($body);
    }

    function fundsPageBody($investorID, $message, $display) {

        // angeldie("fundsPageBody: ");
        $inv = new angelPerson();
        $conn = $inv->connect();
        $inv->id = $investorID;
        if ($investorID != 0)   // just let root be
            $inv =  $inv->findMatchingPerson(NULL);

        $investmentsDisplay = "none";
        $payoutsDisplay = "none";
        $companiesDisplay = "none";
        $valueDisplay = "none";
        $dealflowDisplay = "none";

        if ($display == "" or $display == "investments" or $display == "login" )
            $investmentsDisplay = "block";
        elseif ($display == "payouts")
            $payoutsDisplay = "block";
        elseif ($display = "companies")
            $companiesDisplay = "block";

        if ($inv == NULL) {
            angeldie("angelcalcpage called w/illegal investor ID: " . $investorID);
        }

        $body = $this->createAngelcalcDialogs($investorID, $conn, $message, $display);
        $body .= $this->createFundsLeftNav($investorID);



        $body .= "<div id='companiesContainer' style='display:$companiesDisplay; border: 0px solid black; margin: 0px 0px 0px 220px; padding-left: 2px'>";
        $body .= "</div>";  // companiesContainer

        $body .= "<div id='fundsContainer' style='display:$investmentsDisplay'>";
        $body .= "<div id='transparentHelpDiv' style='opacity:1; color:blue; z-index:99; display:none; width: 250px; position:absolute; top:120px; height:250px; border: 3px dotted green; margin: 0px 0px 0px 220px; float:left; padding: 6px'>";
        $body .= "<-- The easiest way to begin is to click on this '+' sign to add a new investment.<p /><p />
                    Alternatively, to add a number of investments (or distributions) at once, click on 'Import' here:
                  <p />

                    <-- Import

                ";

        $body .= "</div>";  // transparentHelpDiv

        $body .= "<div id='friendHeaderDiv' style='margin: 0px 0px 20px 220px; width: 400px; padding: 20px; border: 2px solid blue; display:none'>Showing <span id='friendHeader'></span>'s Investments </div>";

        $body .= "<div id='investmentsTableHeaderDiv' class='angelbasic' style='border: 0px solid orange; margin: 0px 0px 0px 220px;width:900px;'>
                <table class='angelInvestmentsTable' style='border: 1px solid black;'>
                <thead>
                <tr>
                    <th id='angelInvestsHeaderCol1' mySort='company' myProp='company_id' class='header' valign=top style='width:150px'>Fund</th>
                    <th id='angelInvestsHeaderCol2' mySort='date'    myProp='inv_date'class='header' valign=top style='width:100px'>Date</th>
                    <th id='angelInvestsHeaderCol3' mySort='string'  myProp='round' class='header' valign=top style='width:180px'>Commitment</th>
                    <th id='angelInvestsHeaderCol4' mySort='number'  myProp='invest_amount' class='header' valign=top style='width:100px'>Paid</th>
                    <th id='angelInvestsHeaderCol5' mySort='number'  myProp='shares' class='header' valign=top style='width:100px'>Left to Pay</th>
                    <th id='angelInvestsHeaderCol6' mySort='string'  myProp='notes' class='header' valign=top style='width:260px'>Notes</th>
                    <th class='header' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"investments\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                 </tr></thead></table></div>";

        $body .= "<div id='investmentsTableDiv' class='angelbasic' style='margin: 0px 0px 0px 220px;height: 300px; width:910px;overflow:auto; float:left; border: 1px solid black; padding-left: 2px;display:inline'>";
        $body .= "<table id='angelInvestmentTable' class='angelInvestmentsTable tl1 tl3 tr4 tc5 tl6'>";
        $body .= "  <thead>
                    <tr>
                        <th valign=top style='width:150px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:180px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:260px'></th>
                    </tr></thead><tbody></tbody></table></div>";

        $body .= "</div>";  // investmentsContainer


        $body .= "<div id='dealflowContainer' style='display:$dealflowDisplay; border: 0px solid black; margin: 0px 0px 0px 220px; padding-left: 2px'>";
        $body .= "</div>";  // dealFlowContainer

        $body .= "<div id='valuesContainer' style='display:$valueDisplay'>";
        $body .= "<div id='valuesTableHeaderDiv' class='angelbasic' style='border: 0px solid orange; margin: 0px 0px 0px 220px;width:1200px;'>
                <table class='angelInvestmentsTable' style='border: 1px solid black;'>
                <thead>
                <tr>
                    <th id='angelValuesHeaderCol1' mySort='company' myProp='id' class='header' valign=top style='width:150px'>Company</th>
                    <th id='angelValuesHeaderCol2' mySort='number'  myProp='invest' class='header' valign=top style='width:100px'>Amount</th>
                    <th id='angelValuesHeaderCol3' mySort='number'  myProp='shares' class='header' valign=top style='width:100px'>Shares</th>
                    <th id='angelValuesHeaderCol4' mySort='number'  myProp='sharePrice' class='header' valign=top style='width:100px'>Price</th>
                    <th id='angelValuesHeaderCol5' mySort='number'  myProp='ownership' class='header' valign=top style='width:120px'>Ownership</th>
                    <th id='angelValuesHeaderCol6' mySort='number'  myProp='valuation' class='header' valign=top style='width:150px'>Co Value</th>
                    <th id='angelValuesHeaderCol7' mySort='number'  myProp='yourvalue' class='header' valign=top style='width:150px'>Your Value</th>
                    <th id='angelValuesHeaderCol8' mySort='number'  myProp='yourexitvalue' class='header' valign=top style='color:green;width:150px'>Predicted Value</th>
                    <th class='header' valign=top ><div style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"values\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                 </tr></thead></table></div>";

        $body .= "<div id='valuesTableDiv' class='angelbasic' style='margin: 0px 0px 30px 220px;height: 300px; width:1050px;overflow:auto; float:left; border: 1px solid black; padding-left: 2px;display:inline'>";
        $body .= "<table id='angelValueTable' class='angelValuesTable tl1 tr2 tr3 tr4 tr5 tr6 tr7 tr8'>";
        $body .= "  <thead>
                    <tr>
                        <th valign=top style='width:150px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:100px'></th>
                        <th valign=top style='width:120px'></th>
                        <th valign=top style='width:150px'></th>
                        <th valign=top style='width:150px'></th>
                        <th valign=top style='width:150px'></th>
                        </tr></thead><tbody></tbody></table>
                    </div>";

        $body .= "    <div id='valuesTotalsDiv' style='clear:both;width:360px; margin: 0px 0px 0px 900px; padding: 3px; height:20px; border: 3px solid blue'>
                                                     <div style='float:left'>Totals: </div>
                                                     <div style='float:left; margin:0px 0px 0px 85px;'><span id='angelValueTotal'></span></div>";
        $body .= "                                   <div style='float:right; margin:0px 0px 0px 50px; color:green'><span id='angelPredictedValueTotal'></span></div>
                      </div>";

        if(!isset($_COOKIE["angelcalc-valueInstructions"])) {
            $body .= "       <div id='valueInstructions' style='clear:both; margin: 30px 0px 0px 250px; width:800px; padding:2px; border:2px solid blue'>
                            <a href='javascript:onclick=closeDiv(\"valueInstructions\")'><div class='divWithClose'></div></a>
                            <div style=' padding:15px;'>
                                In order to determine the value of your ownership in a company you need two pieces of
                                information: 1) the number of shares you own (which you may enter by editing each
                                investment in a company) and 2) the latest price per share. The latter is, of course,
                                derived from the company's current valuation divided by the number of fully diluted shares
                                the company currently has outstanding. This is an interesting number because it also tells
                                you your ownership percentage in the company. <p/> You can request the share price and/or
                                valuation and fully diluted shares from the company. These values may be entered
                                directly above by clicking on the <span style='color:blue'>Your Value</span> column in the row for the company you wish
                                to update or
                                on the Company Notes Page by adding a 'Valuation Update' note. These values
                                are always completely private on Angelcalc and are only visible by you.
                                <p/>
                                You may also keep track of your predicted outcome for each company by entering a
                                predicted exit price by clicking in the <span style='color:blue'>Predicted Value</span> of column, or, as above,
                                enter a specific note - 'Exit Prediction' from the Notes page.
                                <p/>
                                You can get to a
                                company's notes page by clicking on its name.
                            </div>

                         </div>";
        }
        $body .= "         </div>";  // valuesContainer


        $body .= "<div id='payoutsContainer' style='display:$payoutsDisplay'>";
        $body .= "<div id='payoutsTableHeaderDiv' class='angelbasic' style='float:left; margin: 0px 0px 0px 220px;width: 1000px;'>";
        $body .= "<table class='angelPayoutsTable'>
                    <thead>
                    <tr>
                        <th id='angelPayoutsHeaderCol1' mySort='company' myProp='companyID' class='header' valign=top style='width:150px'>Company</th>
                        <th id='angelPayoutsHeaderCol2' mySort='date'    myProp='payDate'class='header' valign=top style='width:100px'>Date</th>
                        <th id='angelPayoutsHeaderCol3' mySort='string'  myProp='type' class='header' valign=top style='width:100px'>Type</th>
                        <th id='angelPayoutsHeaderCol4' mySort='number'  myProp='cash' class='header' valign=top style='width:100px'>Cash</th>
                        <th id='angelPayoutsHeaderCol5' mySort='number'  myProp='shares' class='header' valign=top style='width:100px'>Shares</th>
                        <th id='angelPayoutsHeaderCol6' mySort='string'  myProp='symbol' class='header' valign=top style='width:100px'>Symbol</th>
                        <th id='angelPayoutsHeaderCol7' mySort='number'  myProp='sharePrice' class='header' valign=top style='width:100px'>Price</th>
                        <th id='angelPayoutsHeaderCol8' mySort='number'  myProp='value' class='header' valign=top style='width:100px'>Value</th>
                        <th id='angelPayoutsHeaderCol9' mySort='string'  myProp='notes' class='header' valign=top style='width:135px'>Notes</th>
                        <th class='header' valign=top ><div title='print!!' style='margin:0px 0px 0px 30px;'><a href='javascript:onclick=angelPrint(\"payouts\")'><img src='img/printer_icon.png' height='20px' width='20px'></div></a></th>
                     </tr></thead></table></div>";

        $body .= "<div id='payoutsTableDiv' class='angelbasic' style='margin: 0px 0px 0px 220px;height: 300px;width:1000px; overflow:auto; float:left; padding-left: 2px; '>";
        $body .= "<table id='angelPayoutsTable' class='angelPayoutsTable tl1 tr3 tr4 tr5 tr6 tr7 tr8 tr9'>";
        $body .= "  <thead>
                    <tr>
                        <td valign=top style='width:150px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:135px;'></td>
                    </tr></thead><tbody></tbody></table></div>";

        $body .= "</div>";  // payoutsContainer

        $body .= "<div style='clear:both; padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";

        $body .= "<div id='summaryGraph1' class='angelbasic' style='margin: 0px 0px 0px 220px;clear:left; width: 455px; height: 300px; float:left; overflow:auto; background-color: white'></div>";
        $body .= "<div id='summaryGraph2' class='angelbasic' style='margin: 0px 0px 0px 0px;width: 445px; height: 300px; float:left; overflow:auto; background-color: white'></div>";



        $body .=  "<div id='addInvestmentDiv' class='angelAddForm'>";
        $body .=  $this->newInvestmentForm($investorID, $conn);
        $body .= "</div>";
        $body .=  "<div id='addPayoutDiv' class='angelAddForm'>";
        $body .=  $this->newPayoutForm($investorID, $conn);
        $body .= "</div>";
        $body .=  "<div id='addFundInvestDiv' class='angelAddForm'>";
        $body .=  $this->newFundInvestForm($investorID, $conn);
        $body .= "</div>";
        $body .=  "<div id='addFundPayoutDiv' class='angelAddForm'>";
        $body .=  $this->newFundPayoutForm($investorID, $conn);
        $body .= "</div>";
        $body .=  "<div id='addFundPaymentDiv' class='angelAddForm'>";
        $body .=  $this->newFundPaymentForm($investorID, $conn);
        $body .= "</div>";

        // $body .= "<div style='clear:both; padding: 0px 0px 50px 0px; border: 3px solid red;'>&nbsp</div>";

        return $body;
    }       // fundsPageBody

    function createFundsLeftNav($investorID) {
        $leftNav = "";

        $leftNav .= "<div class='angelcalcNav'>

                        <table class='tr2 tr3' style='margin: 0px 0px 0px 7px;width:98%'>
                            <tr><td colspan='2' id='investmentsNav'>     Fund Commitments</td><td id='addInvestmentNav'>+</td></tr>
                            <tr><td colspan='2' id='payoutsNav'>         Distributions</td><td id='addPayoutNav'>+</td></tr>
                            <tr><td colspan='2' id='companiesNav'>      Capital Payments</td><td id='addCompanyNav'>+</td></tr>
                            <tr><td colspan='2' id='valuesNav'>      Portfolio Value</td></tr>

                            <tr><td colspan='3'><hr></td></tr>";
        $friends = angelGetFriends($investorID);
        if (sizeof($friends) != 0) {
            $leftNav .= "<tr><td colspan='2' id='friendNav'>      Friends</td><td id='addFriendNav'>+</td></tr>";
            $leftNav .= "<tr><td class='friendList' colspan='3'><div class='friendList' id='friendList' style='display: none'>";
            foreach ($friends as $f) {
                $name = angelTruncateString($f->firstname . " " . $f->lastname, 16);
                $leftNav .= "<a href='javascript:onclick=showFriendInvestments(\"$f->id\",\"$name\")'><span>$name</span></a><br/>";
            }
            $leftNav .= "<tr><td colspan='3'><hr></td></tr>";
            $leftNav .= "</div></td></tr>";
        }


        $leftNav .= "
                            <tr><td colspan='2' id='importNav'>      Import  </td></tr>
                            <tr><td colspan='2' id='exportNav'>      Export</td></tr>
                            <tr><td colspan='2' id='taxNav'>      Tax</td></tr>
                            <tr><td colspan='3'><hr></td></tr>
                            <tr><td colspan='3' style='color:green'>    Filter</td></tr>
                            <tr><td></td><td onclick='FundsView.switchFilter(\"all\")' id='showAll'>              Show All</td></tr>
                            <tr><td></td><td onclick='FundsView.switchFilter(\"active\")' id='showActive'>           Show Active</td></tr>
                            <tr><td></td><td onclick='FundsView.switchFilter(\"dead\")' id='showClosed'>         Show Acquired</td></tr>
                        </table>

                        <table class='tl1 tr2' style='width:90%; border: 0px solid green; background-color: white;  margin: 20px 0px 0px 7px;'>
                            <tr><td colspan='2' style='color:green'>    Summary</td></tr>
                            <tr><td>            Funds:</td>  <td id='summaryFunds'></td></tr>
                            <tr><td>            Active Funds:</td>   <td id='summaryActive'></td></tr>
                            <tr><td>            Committed:</td><td id='summaryInvestments'></td></tr>
                            <tr><td>            Commit Left:</td>   <td id='summaryInvested'></td></tr>

                            <tr><td colspan='2'><hr></td></tr>

                            <tr><td>            Returned:</td>   <td id='summaryReturned'></td></tr>
                        </table>

                        <div style='margin: 0px 0px 0px 0px' id='messageDiv'></div>



        </div>";
        /*
         *                        <div style='margin: 40px 2px 0px 7px;border: 0px solid green;height:300px;' id='historyDiv'>
                        <table id='historyTable' class='tl1 tr2' style='width:100%; border: 2px solid green;'>
                            <tr><td colspan='2' style='color:green'>    History</td></tr>
                            <tr><td style='font-size:10px'>9-25-16 (2:51pm)</td>   <td style='font-size:10px'>New invest in XYZ</td></tr>
                        </table>
                        </div>
         */
        return($leftNav);
    }

    /**************************************************************************************
     * Funds - end
     */

    function investmentSummary($conn, $id) {
        $ai = new angelInvest();
        $ai->shareconn($conn);
        $r = $ai->getRows("angelinvests", "investor_id=$id");

        $numInvestments = 0;
        $totalSpent = 0;
        while($inv = mysqli_fetch_object($r,"angelInvest")){
            $numInvestments++;
            $totalSpent += $inv->invest_amount;
        }
        $spent = sprintf("$%01.3fmm", $totalSpent/1000000);
        $summary = "<p></p>$numInvestments Investments / $spent spent<p></p>";
        return($summary);

    }
    /*
    * Display investments for the input used (id)
    */
    function showInvestmentsByInvestor($conn, $id) {
        $numInvestments = 0;
        $path = $GLOBALS["path"];

        // $investors = getUniqueInvestors($conn, $co);

        $query = "SELECT * from angelinvests WHERE investor_id=$id ORDER BY inv_date DESC";
        //angeldie($query);    // debug
        $r = mysqli_query($conn, $query);
        if (!$r)
            angeldie("Investment->showInvestmentsByInvestor select investmenttable Failed: " . mysqli_error($conn));

        $Investments = "<table id='angelInvestmentTable' style='table-layout: fixed; width:650px'>";

        $Investments .= "<thead>
        <tr>
                    <th valign=top style='width:100px'></th>
                    <th valign=top style='width:150px'></th>
                    <th valign=top style='width:100px'></th>
                    <th valign=top style='width:100px'></th>
                    <th valign=top style='width:100px'></th>
                    <th valign=top style='width:100px'></th>
                 </tr></thead>";


        $spent = 0;

        while($int = mysqli_fetch_object($r,"angelInvest")){
            /*
             * handle case where there was a conversion
             */
            if ($int->convert_id and ($int->type == "safe" or $int->type == "note")) {
                /*
                 * skip the original and only show the equity
                 */
                continue;
            }
            /*******/
            $numInvestments++;
            $amount = sprintf("$%sk", $int->invest_amount/1000);
            if ($int->type == "safe" or $int->type == "note")
                $value = sprintf("$%01.2fmm", $int->cap/1000000);
            else
                $value = sprintf("$%01.2fmm", $int->prevaluation/1000000);

            $co = new angelCompany();
            $co->id = $int->company_id;
            $co->shareconn($conn);
            $co = $co->findMatchingCompany();

            $val = $this->getValuation($int);

            $Investments .=  "<tr>
                                     <td><a href='$path/investments?edit&investID=$int->id'>$int->inv_date</a></td>
                                     <td><a href='$path/companypage?id=$int->company_id'>$co->name</a></td>
                                     <td>$int->round";
            if ($int->type == "safe" | $int->type == "note")
                $Investments .=  "(<a href='/investments?convert&investID=$int->id'>$int->type</a>)  @ $value cap";
            else {
                if ($int->type == "")
                    $Investments .=  "($int->type) @ $value";
                else
                    $Investments .=  "($int->type) @ $value";
            }

            $Investments .=  "

                                     <td>$amount</td>
                                     <td>$val</td>
                                     <td  style='width:25%; text-align:left;'>$int->notes</td></tr>";

            $spent += $int->invest_amount;

        }
        // $Investments .= "</table><a href='/Investments?add=$co->id'>New Investment</a>";
        $spent = sprintf("$%01.3fmm", $spent/1000000);

        return("<tr><td>$Investments</td></tr><tr><td colspan='7' style='text-align:left; border-top:1px solid black; column-span: 3'>$numInvestments Investments / $spent spent</td></tr></table>");
    }

    /*
     * Return an estimate of the current value of the company referenced in the input investment
     */
    function getValuation($inv) {
        return("who knows??");

    }
    function ImportResultsBody($addedCount, $skippedCount, $lines, $questionableMatches) {
        $path = $GLOBALS["path"];
        /*
         * Header
         */

        $body = "<span style='font-size: larger'>";

        $body .=  "Import Results</span><br><br>";

        $body .= "<div id='content' style='border:2px solid black; width: 80%; height: 400px; overflow:auto'>";

        $body .= "<table>";
        $body .= "<tr><td style='color:blue'>$addedCount entries imported. $skippedCount line(s) skipped.</td></tr>";

        $body .= "<tr><td><hr></td></tr>";
        if ($questionableMatches != "") {
            $body .= "<tr><td style='color:red'><span style='color:blue'>Please look closely at the following company name pairs.
                                            The left side of each pair is the company from the investment row, and the right side
                                            is the company we matched it to. We try to be flexible in our matching, but it is possible
                                            we got a few wrong. These ones are particularly likely to be incorrect. If so, it is simple
                                            in the home/investment view to edit your investment and then choose or add the correct company:


            </span><br /> $questionableMatches</td></tr>";
        }
        $body .= "<tr><td><hr></td></tr>";

        foreach ($lines as $l) {
            $body .= "<tr><td>$l</td></tr>";
        }
        $body .= "</table> </div>";
        $body .= "<a href='$path/investments'>Continue to Investments</a>";

        return $body;
    }
    /******************************************
     * PayoutsPage Body -- DEPRECATED
     *
     * @param $investorID
     * @return string
     *
     */

    function PayoutsPageBody($investorID) {


        $inv = new angelPerson();
        $conn = $inv->connect();
        $inv->id = $investorID;
        $inv =  $inv->findMatchingPerson(NULL);

        $path = $GLOBALS["path"];

        /*
         * Header
         */

        $body = "<span style='font-size: larger'>" . $inv->firstname . "'s Payouts</span><p></p>";

        $body .= "
        <div id='dialogDelete' title='Delete Payout' style='display:none'>
                                            <form id='' action='' method='post' enctype='multipart/form-data'>
                                                 Permanently delete this distribution from <span id='payoutToDelete'></span>?
                                            </form>
                                    </div>";


        $addDivDisplay = "none";
        $upArrowDisplay = "inline";
        $downArrowDisplay = "none";

        $x = "initializeInvestments($investorID)";
        $body .= "<script>$(document).ready( function() { $x } );</script>";

        $body .= "<div class='angelbasic' style='margin: 0px 0px 0px 0px; border: 1px solid black; width:1000px'>
                <table class='angelInvestmentsTable'>
                <thead>
                <tr>
                    <th id='angelPayoutsHeaderCol1' mySort='company' myProp='companyID' class='header' valign=top style='width:150px'>Company</th>
                    <th id='angelPayoutsHeaderCol2' mySort='date'    myProp='payDate'class='header' valign=top style='width:100px'>Date</th>
                    <th id='angelPayoutsHeaderCol3' mySort='string'  myProp='type' class='header' valign=top style='width:100px'>Type</th>
                    <th id='angelPayoutsHeaderCol4' mySort='number'  myProp='cash' class='header' valign=top style='width:100px'>Cash</th>
                    <th id='angelPayoutsHeaderCol5' mySort='number'  myProp='shares' class='header' valign=top style='width:100px'>Shares</th>
                    <th id='angelPayoutsHeaderCol6' mySort='number'  myProp='sharePrice' class='header' valign=top style='width:100px'>Price</th>
                    <th id='angelPayoutsHeaderCol7' mySort='string'  myProp='notes' class='header' valign=top style='width:135px'>Notes</th>
                 </tr></thead></table></div>";

        $body .= "<div class='angelbasic' style='height: 300px; overflow:auto; float:left; width:1000px'>";
        $body .= "<table id='angelPayoutsTable' class='angelInvestmentsTable tl1 tl3 tr4 tr5 tr6'>";
        $body .= "  <thead>
                    <tr>
                        <td valign=top style='width:150px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:100px'></td>
                        <td valign=top style='width:135px'></td>
                    </tr></thead><tbody></tbody></table>";
        $body .= "</div>";

        /*
         * add a graph if you want!

        $body .= "<div id='contentSummary' class='angelbasic' style='width: 500px; height: 300px; overflow:auto; background-color: white'>";
        $body .= "</div>";
        */

        $body .= "</div><div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'></div>";  // container and clear
        $body .= "<div style='clear:both;border-top: 2px dashed gray; width: 100%; margin: 30px 0px 0px 0px;'>";
        $body .= "<p id='note'><a href='#' width='10' class='add' style='display: $upArrowDisplay;' ><img src='/img/up_arrow.png'></a>
                         <a href='#'  width='10' class='add' style='display: $downArrowDisplay;'><img src='/img/down_arrow.png'></a><b>&nbsp;&nbsp; <a href='#'>Add Payout</a></b></p>";
        $body .=  "<div id='addInvestmentDiv' class='add' style='display: $addDivDisplay;'>";


        $body .= $this->newPayoutForm($investorID, $conn);
        return $body;
    }

    function PerfPageBody($investorID) {


        $inv = new angelPerson();
        $conn = $inv->connect();
        $inv->id = $investorID;
        $inv =  $inv->findMatchingPerson(NULL);

        /*
         * Header (IK12 [cohort chooser] Investments
         */

        $body = "<span style='font-size: larger'>";

        $body .=  $inv->firstname . "'s Investment Performance
                                                     </span><br><br>";
        if ($this->error) {
            // create an error div. also, we need to restore all the values! damn //
            $body .= $this->error;
        }


        $body .= "<div id='content' class='angelinput' style='border:2px solid black; width: 80%; height: 300px; overflow:auto'>";
        $body .=        $this->showPerfByInvestor($conn, $investorID);
        $body .= "
                    </div>
                ";

        return $body;
    }

    /*
 * Display investments for the input used (id)
 */
    function showPerfByInvestor($conn, $id) {
        $numInvestments = 0;
        $totalValue = 0;
        $totalInvested = 0;

        $path = $GLOBALS["path"];

        // $investors = getUniqueInvestors($conn, $co);
        $inv = new angelInvest();
        $inv->shareconn($conn);

        $r = $inv->getRows($inv->tableName(), "investor_id=$id");

        if (!$r)
            angeldie("Investment->showPerfByInvestor select angelinvests Failed: " . $inv->error());

        $Investments = "<table id='angelInvestmentTable'>";
        $Investments .= "<thead>
        <tr>
            <th valign=top>Date</th>
            <th valign=top>Company</th>
            <th valign=top>Amount Invested</th>
            <th valign=top>Valuation</th>
            <th valign=top>Price Paid</th>
            <th valign=top>Current Valuation</th>
            <th valign=top>Latest Price</th>
            <th valign=top>Your Value</th>
            <th valign=top>Gain (loss)</th>

         </tr></thead>";
        $spent = 0;
        while($int = mysqli_fetch_object($r,"angelInvest")){
            /*
             * handle case where there was a conversion
             */
            if ($int->convert_id and ($int->type == "safe" or $int->type == "note")) {
                /*
                 * skip the original and only show the equity
                 */
                continue;
            }
            /*******/
            $numInvestments++;
            $amount = sprintf("$%sk", $int->invest_amount/1000);
            if ($int->type == "safe" or $int->type == "note")
                $value = sprintf("$%01.2fmm", $int->cap/1000000);
            else
                $value = sprintf("$%01.2fmm", $int->prevaluation/1000000);
            $currentValue = sprintf("$%01.2fmm", $int->current_valuation/1000000);
            $co = new angelCompany();
            $co->id = $int->company_id;
            $co->shareconn($conn);
            $co = $co->findMatchingCompany();

            $Investments .=  "<tr>
                                     <td><a href='$path/investments?edit&investID=$int->id'>$int->inv_date</a></td>
                                     <td><a href='$path/companypage?id=$int->company_id'>$co->name</a></td>
                                     <td>$amount</td>
                                     <td>$value</td>
                                     <td>$int->price_paid</td>
                                     <td>$currentValue</td>
                                     <td>$int->price_share</td>

                                     ";
            $yourValue = ($int->shares * $int->price_share) - ($int->shares * $int->price_paid);
            $gain = $yourValue - $amount;
            $yourValue = sprintf("$%01.2fmm", $yourValue/1000000);
            $gain = sprintf("$%01.2fmm", $gain/1000000);
            $Investments .=  "
                                     <td>$yourValue</td>
                                     <td>$gain</td>
                                     </tr>";
            $spent += $int->invest_amount;

        }
        // $Investments .= "</table><a href='/Investments?add=$co->id'>New Investment</a>";
        $spent = sprintf("$%01.3fmm", $spent/10000000);

        $Investments .=  "</td></tr><tr><td colspan='9' style='text-align:left; border-top:1px solid black;'>";
        $Investments .= $numInvestments . " Investments / " . $spent . " spent</td></tr>";
        $Investments .=  "</td></tr><tr><td colspan='9' style='text-align:left; border-top:1px solid black;'></td></tr>";
        $Investments .= "</table>";


        return($Investments);
    }
    /*
     * Returns the price of the input company (id) at the input date (what format??). If date is null, then the most
     * current price is returned.
     */
    function stockPrice($co, $date) {

    }


    function ImportPageBody($investorID) {

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        $page .= "<div style='margin: 80px 0px 0px 0px'>";
        $page .= "   &nbsp <a href='#' style='color:black' id='importInvestments' onclick='importChooser(this,\"investments\")'>Investment file import</a> |
                    <a href='#' id='importPayouts'onclick='importChooser(this,\"payouts\")' >Distribution file import</a>";
        if (isRoot($investorID))
            $page .= " |  <a href='#' id='importYCFile' onclick='importChooser(this,\"ycfile\")'>YC file import</a>";
        $page .= "</div>";

        $page .= "  <div class='importContainer'>
                        <form action='/investments.php' method='post' enctype='multipart/form-data'>
                            Select CSV file to upload:
                            <input type='file' name='csvfile' id='fileUpload'>
                            <input type='submit' value='Upload' name='csvUpload'>
                            <input type='hidden' id='uploadType' name='uploadType' value='investments'>
                            <div id='uploadSkipDiv'>
                            <p /><input type='checkbox' id='uploadSkip' name='uploadSkip' value='Y' checked>
                                <span style='font-weight: 100; font-size: 12px'>Check here to ignore
                                lines matching existing investments of the same  amount in
                                the same company on the same day.
                                Only new investments/distributions will be added and
                                updates made to investments/distributions within Angelcalc (e.g. a conversion)
                                will be preserved.</span></div>
                        </form>
                    </div>";

        $page .= "<div class='importContainer' id='importInvestmentsFormat'>
                       <span style='color:blue'>Investment CSV File Format. The first column below identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>              <th>Definition</th></tr>
                            <tr style='color:red'><td>Date</td>             <td>Investment Date</td>	</tr>
                            <tr style='color:red'><td>Company</td>          <td>Name of the company</td>	</tr>
                            <tr style='color:red'><td>Amount</td>           <td>Amount invested in the company</td>	</tr>
                            <tr><td>Round</td>                              <td>One of: {Seed, Series A, Series B, Series D, Bridge, Acquisition, IPO,Other}</td>	</tr>
                            <tr><td>Valuation</td>                          <td>Valuation cap (safe or note) or pre-money valuation of company</td>	</tr>
                            <tr><td>Type</td>                               <td>One of: {Note, Safe, Equity, Other}</td>	</tr>
                            <tr style='color:red'><td>Status</td>           <td>One of: {Active, Dead, Acquired, IPO, Unknown}</td>	</tr>
                            <tr><td>PrefShares</td>                         <td>Number of Preferred shares you purchased in the company</td>	</tr>
                            <tr><td>PrefCert</td>                           <td>Certificate number of your preferred shares</td>	</tr>
                            <tr><td>CommShares</td>                         <td>Number of Common shares you purchased in the company</td>	</tr>
                            <tr><td>CommCert</td>                           <td>Certificate number of your comon shares</td>	</tr>
                            <tr><td>Notes</td>                              <td>Misc notes</td>	</tr>
                            <tr><td>Batch</td>                              <td>Indicate the batch if a Y Combinator or Imagine K12 Company (e.g. w2017 or ik12 f2015)</td>	</tr>
                            <tr><td>Converted</td>                          <td>'Yes' or 'Y' indicates this was originally a convertible which converted to shares</td>	</tr>
                            <tr><td>QSB</td>                                <td>'Yes' or 'Y' indicates you believe this company is a qualified small business</td>	</tr>

                       </table>
                    </div>";

        $page .= "<div class='importContainer' id='importPayoutsFormat' style='display:none;'>
                       <span style='color:blue'>Distribution CSV File Format. The first column below identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>  <th>Definition</th></tr>
                            <tr style='color:red'><td>Date</td>         <td>Distribution Date</td>	</tr>
                            <tr style='color:red'><td>Company</td>      <td>Name of the company acquired (or IPO'd)</td>	</tr>
                            <tr style='color:red'><td>Type</td>         <td>One of: {Cash or stock} </td>	</tr>
                            <tr style='color:red'><td>Cash</td>         <td>Amount being paid in cash</td>	</tr>

                            <tr><td>Shares</td>                         <td>Number of shares in stock distribution </td>	</tr>
                            <tr><td>Symbol</td>                         <td>Symbol or name (if not public) of shares in stock distribution</td>	</tr>
                            <tr><td>Price</td>                          <td>Per Share price at distribution time (average on day) if a stock dist</td>	</tr>
                            <tr><td>Notes</td>                          <td>Misc notes</td>	</tr>

                       </table>
                   </div>";

        $page .= "<div class='importContainer' id='importYCFileFormat' style='display:none;'>
                       <span style='color:blue'>YC CSV File Format. The first column below identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>          <th>Definition</th></tr>
                            <tr style='color:red'><td>ID</td>           <td>YC ID</td>	</tr>
                            <tr style='color:red'><td>Name</td>         <td>Company Name</td>	</tr>
                            <tr style='color:red'><td>Batch</td>        <td>YC Batch</td>	</tr>
                            <tr style='color:red'><td>Description</td>  <td>Brief Description </td>	</tr>
                            <tr style='color:red'><td>Sector</td>       <td>Company Sector</td>	</tr>
                            <tr style='color:red'><td>Status</td>       <td>Current status: active, dead, acquired, IPO, or adrift</td>	</tr>
                       </table>
                   </div>";


        return($page);

    }       // importPageBody

    function ImportFundsPageBody($investorID) {

        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        $page .= "<div style='margin: 80px 0px 0px 0px'>";
        $page .= "  &nbsp  <a href='#' style='color:black' id='importFundInvestments' onclick='importFundChooser(this,\"fundinvestments\")'>Fund Investment file import</a> |
                    <a href='#' id='importFundPayouts'onclick='importFundChooser(this,\"fundpayouts\")' >Fund Distribution file import</a>";
        $page .= "</div>";

        $page .= "  <div class='importContainer'>
                        <form action='/investments.php' method='post' enctype='multipart/form-data'>
                            Select CSV file to upload:
                            <input type='file' name='csvfile' id='fileUpload'>
                            <input type='submit' value='Upload' name='csvUpload'>
                            <input type='hidden' id='uploadType' name='uploadType' value='fundinvestments'>
                            <div id='uploadSkipDiv'>
                            <p /><input type='checkbox' id='uploadSkip' name='uploadSkip' value='Y' checked>
                                <span style='font-weight: 100; font-size: 12px'>Check here to ignore
                                lines matching existing investments of the same  amount in
                                the same company on the same day.
                                Only new investments/distributions will be added and
                                updates made to investments/distributions within Angelcalc (e.g. a conversion)
                                will be preserved.</span></div>
                        </form>
                    </div>";

        $page .= "<div class='importContainer' id='importFundInvestmentsFormat'>
                       <span style='color:blue'>Investment CSV File Format. The first column below identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.
                       Note, for an investment in a fund to be accepted, there MUST be an existing investment committment (type=commit) for
                       that fund.
                       </span>

                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>              <th>Definition</th></tr>
                            <tr style='color:red'><td>Date</td>             <td>Investment Date</td>	</tr>
                            <tr style='color:red'><td>Fund</td>             <td>Name of the Fund</td>	</tr>
                            <tr style='color:red'><td>Amount</td>           <td>Amount invested or committed to the fund</td>	</tr>
                            <tr style='color:red'><td>Type</td>                               <td>One of: {commit or invest}</td>	</tr>
                            <tr><td>Notes</td>                              <td>Misc notes</td>	</tr>

                       </table>
                    </div>";

        $page .= "<div class='importContainer' id='importFundPayoutsFormat' style='display:none;'>
                       <span style='color:blue'>Distribution CSV File Format. The first column below identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>  <th>Definition</th></tr>
                            <tr style='color:red'><td>Date</td>         <td>Distribution Date</td>	</tr>
                            <tr style='color:red'><td>Fund</td>      <td>Name of the fund making the distribution</td>	</tr>
                            <tr style='color:red'><td>Type</td>         <td>One of: {Cash or stock} </td>	</tr>
                            <tr style='color:red'><td>Cash</td>         <td>Amount being paid in cash</td>	</tr>

                            <tr><td>Shares</td>                         <td>Number of shares in stock distribution </td>	</tr>
                            <tr><td>Symbol</td>                         <td>Symbol or name (if not public) of shares in stock distribution</td>	</tr>
                            <tr><td>Price</td>                    <td>Per Share price at distribution time (average on day) if a stock dist</td>	</tr>
                            <tr><td>Notes</td>                          <td>Misc notes</td>	</tr>

                       </table>
                   </div>";

        $page .= "<div class='importContainer' id='importYCFileFormat' style='display:none;'>
                       <span style='color:blue'>YC CSV File Format. The first column below identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>          <th>Definition</th></tr>
                            <tr style='color:red'><td>ID</td>           <td>YC ID</td>	</tr>
                            <tr style='color:red'><td>Name</td>         <td>Company Name</td>	</tr>
                            <tr style='color:red'><td>Batch</td>        <td>YC Batch</td>	</tr>
                            <tr style='color:red'><td>Description</td>  <td>Brief Description </td>	</tr>
                            <tr style='color:red'><td>Sector</td>       <td>Company Sector</td>	</tr>
                            <tr style='color:red'><td>Status</td>       <td>Current status: active, dead, acquired, IPO, or adrift</td>	</tr>
                       </table>
                   </div>";


        return($page);

    }       // importFundsPageBody

    function ExportPageBody() {


        $page = "   <a href='#' style='color:black' id='exportInvestments' onclick='exportChooser(this,\"investments\")'>Investment file export</a> |
                    <a href='#' id='exportPayouts'onclick='exportChooser(this,\"payouts\")' >Distribution file export</a>";

        $page .= "  <div style='border: 1px solid blue; margin: 30px 350px 0px 0px; padding: 30px 30px 30px 30px; width: 50%'>
                        <form action='javascript:exportInvestments();'>
                            <input type='submit' value='Export' name='csvExport'>
                            <input type='hidden' id='exportType' value='investments'>
                        </form>
                    </div>";

        $page .= "<div class='angelbasic' id='exportInvestmentsFormat' style='border: 1px solid blue; margin: 30px 350px 0px 0px; padding: 10px 30px 30px 30px; width: 50%;text-align: center;'>
                       <span style='color:blue'>Investment CSV File Format. The first row identifies the columns/header.
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>              <th>Definition</th></tr>
                            <tr style='color:black'><td>Date</td>           <td>Investment Date</td>	</tr>
                        <tr style='color:black'><td>Company</td>            <td>Name of the company</td>	</tr>
                    <tr style='color:black'><td>Amount</td>                 <td>Amount invested in the company</td>	</tr>
                            <tr><td>Round</td>                              <td>One of: {Seed, Series A, Series B, Series D, Bridge, Acquisition, IPO,Other}</td>	</tr>
                            <tr><td>Valuation</td>                          <td>Valuation cap (safe or note) or pre-money valuation of company</td>	</tr>
                            <tr><td>Type</td>                               <td>One of: {Note, Safe, Equity, Other}</td>	</tr>
                        <tr style='color:black'><td>Status</td>             <td>One of: {Active, Dead, Acquired, IPO, Unknown}</td>	</tr>
                            <tr><td>Preferred Shares</td>                   <td>Number of preferred shares you own in the company</td>	</tr>
                            <tr><td>Common Shares</td>                      <td>Number of common shares you own in the company</td>	</tr>
                            <tr><td>Pref Cert</td>                          <td>Certificate number of the preferred shares you own</td>	</tr>
                            <tr><td>Comm Cert</td>                          <td>Certificate number of the common shares you own</td>	</tr>
                            <tr><td>Notes</td>                              <td>Misc notes</td>	</tr>

                       </table>
                    </div>";

        $page .= "<div class='angelbasic' id='exportPayoutsFormat' style='display:none; border: 1px solid blue; margin: 30px 350px 0px 0px; padding: 10px 30px 30px 30px; width: 50%;text-align: center;'>
                       <span style='color:blue'>Distribution CSV File Format. The first row identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>  <th>Definition</th></tr>
                            <tr style='color:red'><td>Date</td>                   <td>Distribution Date</td>	</tr>
                            <tr style='color:red'><td>Company</td>                <td>Name of the company acquired (or IPO'd)</td>	</tr>
                            <tr><td>Distributing Company</td>                           <td>Name of the company making the payout if not IPO</td>	</tr>
                            <tr style='color:red'><td>Type</td>                   <td>One of: {Cash or stock} </td>	</tr>
                            <tr style='color:red'><td>Cash</td>                   <td>Amount being paid in cash - required if cash payout</td>	</tr>

                            <tr style='color:red'><td>Shares</td>                 <td>Number of shares in distribution - required if stock payout</td>	</tr>
                            <tr><td>Share Price</td>                              <td>Per Share price at distribution time if a stock dist</td>	</tr>

                       </table>
                   </div>";


        return($page);

    }       // export page body

    function TaxPageBody() {
    }

    function ImportPayoutPageBody() {

        $path = $GLOBALS["path"];

        $page = "   <H1>Distribution file import. File must be in comma separated format. </H1>
                    <div style='border: 1px solid blue; margin: 30px 350px 0px 0px; padding: 30px 30px 30px 30px; width: 50%'>
                        <form action='$path/investments.php' method='post' enctype='multipart/form-data'>
                            Select CSV file to upload:
                            <input type='file' name='csvfile' id='fileUpload'>
                            <input type='submit' value='Upload' name='csvPayoutUpload'>
                        </form>
                    </div>";

        $page .= "<div class='angelbasic' style='border: 1px solid blue; margin: 30px 350px 0px 0px; padding: 10px 30px 30px 30px; width: 50%;text-align: center;'>
                       <span style='color:blue'>CSV File Format. The first row identifies the columns. They may be in any order. Columns in <span style='color:red'>red</span> are required.</span>
                       <hr>
                       <table class='tl1 tl2' style='width:100%'>
                            <tr><th>First Row (column id)</th>  <th>Definition</th></tr>
                            <tr style='color:red'><td>Date</td>                   <td>Distribution Date</td>	</tr>
                            <tr style='color:red'><td>Company</td>                <td>Name of the company acquired (or IPO'd)</td>	</tr>
                            <tr><td>Distributing Company</td>                           <td>Name of the company making the payout if not IPO</td>	</tr>
                            <tr style='color:red'><td>Type</td>                   <td>One of: {Cash or stock} </td>	</tr>
                            <tr style='color:red'><td>Cash</td>                   <td>Amount being paid in cash - required if cash payout</td>	</tr>

                            <tr style='color:red'><td>Shares</td>                 <td>Number of shares in distribution - required if stock payout</td>	</tr>
                            <tr><td>Share Price</td>                              <td>Per Share price at distribution time if a stock dist</td>	</tr>

                       </table>


        </div>";

        return($page);

    }


    function HelpPageBody($investor, $whatHelp) {
        // angeldie("helpPageBody: (investor,whathelp): " . angelParens($investor, $whatHelp));     // debug

        $p = new angelPerson();
        if ($investor == 0 or $investor == -1) {
            $p->firstname = "Guest";
        } else {
            $conn = $p->connect();
            $p->id = $investor;
            $p =  $p->findMatchingPerson(NULL);
        }
        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investor) } );</script>";
        $page .= "<div id='helpContainer' style='margin: 20px 20px 20px 20px; padding: 20px 20px 20px 20px'>";
        if ($whatHelp == "beta" or $_SESSION['isBetaUser'] == true) {
            $page .= '  <H1>Angelcalc Beta Program</H1>
                        <div style="border: 1px solid blue; margin: 30px 350px 30px 0px; padding: 30px 30px 30px 30px">';

            $page .= "                 $p->firstname, welcome to Angelcalc! Thanks for agreeing to check it out. Angelcalc is, more than
                                        anything, a labor of love. Please excuse all the warts my baby currently has, but also please let
                                        me know about them. You can use the feedback widget on lower right of most every page or just
                                        email support@angelcalc.com with any questions / bugs. Or you can call me at (650) 533 3331.

                                        Hope you like it!
                        </div>";
            $page .= file_get_contents("includes/angelcalcHelp.inc");

        } else {
             $page .= '  <H1>Angelcalc Help</H1>
                        <div style="border: 1px solid blue; margin: 30px 350px 30px 0px; padding: 30px 30px 30px 30px">';

             $page .= "                 $p->firstname: Glad you are asking for help. For now, we have the instructions below.
                                        Eventually there will be more help, but
                                        in the meantime, please feel free use the feedback widget on the lower right below or
                                        email support@angelcalc.com with any questions / bugs.
                        </div>";
            if ($whatHelp == "model" or $whatHelp == "start") {
                $page .= file_get_contents("includes/helpInstructions.inc");
            } else
                $page .= file_get_contents("includes/angelcalcHelp.inc");
        }

        $page .= "</div>";
        return($page);

    }
    /*****************************************************************************
     *    Called to display a shared model
     *
     * @param $investorID  -- logged in user id of investor
     * @param $investID -- if this is a conversion, id of investment to be converted (or 0)
     * @param $modelID -- shared model to display or 0.
     * @param $hash --  if model != 0, this must match the hash in the model
     * @param $action -- gives path back - either investments or model (the latter if modeling, the former if converting)
     * @return string
     */

    function sharedModelPageBody($modelID, $hash, $action) {
        $path = $GLOBALS["path"];

        $page =  "<div style='float:left'>";
        $mod = new angelModel();
        $mod->id = $modelID;
        $conn = $mod->connect();

        $modelInput = $mod->getEntry();    // get entry of the input model

        if ($modelInput->sharing != $hash ) {
            angeldie("This URL is no longer valid. I'm afraid I can't display the model.");
        }
        $x = "loadModelByID($modelID,'','$hash')";
        $p = new angelPerson();
        $p->shareconn($conn);
        $p->id = $modelInput->investor_id;
        $p = $p->findMatchingPerson(false);
        if ($p == NULL)
            angeldie("Something went wrong: this URL is no longer valid. I'm afraid I can't display the model.");

        $page .= "<div id='whatever' value='$modelID'><script>$(document).ready( function() { $x } );</script></div>";
        $page .= "<span style='font-size:20px; font-weight: bold '>$p->firstname is sharing a model named '$modelInput->name' with you!
                 Click <a href='javascript:onclick=saveModel(\"\")'>save</a> to make a copy for yourself.
        </span>";

        $page .= $this->convertPageOutput();

        $page .= "<div style='clear:both; margin: 5px 0px 5px 45px'><a href='javascript:onclick=modelEquity()' class='modelButton'>Model</a>
                            <a href='javascript:onclick=saveModel(\"\")' class='modelButton' id='saveButton'>Save</a>
                          <span style='margin: 0px 0px 0px 100px;color:white'>
                            <a href='javascript:onclick=modelInitialCap()' class='modelButton'>Show Initial Cap Table</a>
                            <a class='modelButton' href='javascript:onclick=modelConvertOnly()'>Show Cap Table pre-Equity</a>
                            <a href='javascript:onclick=modelEquity()' class='modelButton'>Show Cap Table post-Equity</a>

                            </span>
                            <a title='What do these buttons do?'
                                onClick=window.open('/static/modelbuttonhelp.html','CO','width=700,height=700,0,status=0,location=0,');
                                href='javascript:void(0);'><big><strong>?</strong></big></a>
                                </div>
                            ";



        $page .= $this->modelInputForm("");

        return($page);


    }

    function whoopieTesterPageBody() {
        $body = "";

        $body .= "<div id='whoopieTesterDiv' style='width:500px; border: 2px solid blue; margin: 100px 0px 0px 300px'>";       

        $body .=    "<table class='whoopieGameChooserTable'>";
        $body .=        "<tr><th onclick='whoopieSendRequest(\"resetGame\"); alert(\"Game Reset\")' colspan=4>Reset game</th></tr>";
        $body .=        "<tr><th onclick='whoopieTestStart();  alert(\"Game On!\")' colspan=4>Start game</th></tr>";
        $body .=        "<tr> <th>Choose Event</th> <th>Choose Player</th><th>Enter Card</th><th>Make a Bid</th></tr>";
        $body .= "<form id='testWhoopieForm' action='/whoopie.php' method='post'>";
        $body .=         "<tr><td><select id='testEventName' name='testEventName'>";
        $body .=                    "<option value='' selected disabled>Event to Send </option>
                                     <option value='joinGame'>  joinGame </option>
                                     <option value='newDeal'>      deal </option>
                                     <option value='playCard'>  playCard </option>
                                     <option value='newBid'>       bid </option>

                                     </select></td>";

        $body .=         "<td><select id='testEventPlayer' name='testEventPLayer'>";
        $body .=                                                "<option value='' selected disabled>Choose a Player</option>
                                                                <option value='3'>      Elizabeth</option>
                                                                <option value='2'>      Steven</option>
                                                                <option value='0'>      Geoff</option>
                                                                <option value='1'>      Jon</option>

                                                             </select>


                          </td>";

        $body .=      "<td><input type='text' id='whoopieTestCard'></td>";
        $body .=      "<td><input type='text' id='whoopieTestBid'></td></tr>";
        $body .=      "<div style='margin: 100px 0px 0px 0px; font-size: 20px;'><a href='javascript:onclick=testWhoopie()' class='modelButton'>Send Event</a></div>";

        $body .= "</form></table> </div>";

        return($body);

    }
    
    function whoopieHomePageBody() {

        $game = new whoopieGame();
        $conn = $game->connect();


        $body = "<div style='overflow:hidden'>";

        $body .= $this->createWhoopieDialogs();
        

        /*
         * whoopie home page
         * 
         */

        /*
         * This div is where the game to be played is chosen
         */
        $body .= "<div id='whoopieGameChooser' style='width:500px; border: 2px solid blue; margin: 100px 0px 0px 300px'>";        // choose game div

        $body .=    "<table class='whoopieGameChooserTable'>";

        $rows = $game->getRows("games", "id > 0" );
        $body .=        "<tr><th onclick='whoopieSendRequest(\"resetGame\"); alert(\"Game Reset\")' colspan=3>Choose a game - click on the name!</th></tr>";
        $body .=        "<tr><th>Name</th> <th>Creator</th><th>Description</th></tr>";
        while($g = mysqli_fetch_object($rows,"whoopieGame")) {      
            $body .=        "<tr><td onclick='joinGame(\"$g->name\", $g->firstlogid)'>$g->name</td> 
                                 <td onclick='joinGame(\"$g->name\", $g->firstlogid)'>$g->creator</td>
                                 <td onclick='joinGame(\"$g->name\", $g->firstlogid)'>$g->description</td>
                             </tr>";
        }
        
        $body .=    "</table>";
        $body .= "</div>";

        /*
         * This div is where the game is played xxx
         */
        $body .= "<div id='whoopieTable' style='display:none; background-image:url(/img/cardtable.jpg); border: 0px solid blue; 
                    width:700px; height: 400px; margin: 50px 0px 0px 20px; float:left'>";
        

        /*$body .= "<div id='whoopieMessageDiv' style='display:none; background-color: gray; margin: 250px 0px 0px 315px;width:320px; height: 50px;'>
                 <div style='color:white;   margin: 0px 0px 0px 10px;font-size:28px'><span id='whoopieMessage'>xxxx</span></div></div>";
*/

      
        $body .= "   <div id='dealerButton' style='display:none;'>";

        $body .= "   <img  src='img/dealerbutton2.jpg' height='30px' width='30px'>";

        $body .= "   </div>";



        for ($i=0; $i < 10; $i++ ) { 
            $body .= "<div id='player$i' style='display:none'><img id='playerIMG$i' src='img/founder.jpeg' height='50px' width='50px'>";

            $body .= "<table class='whoopieBidTrickTable'>
                        <tr>
                            <td>B</td><td>T</td> 
                        </tr>
                        <tr>
                             <td id='bidID$i'></td><td id='trickID$i'></td>
                        </tr>";
        
            $body .= "</table></div>";
        }
        
        $body .= "</div>";


        /*
         * This div is where the game is scored
         */
        $body .= "<div id='whoopieScore' style='display:none; border: 0px solid red; width:400px; height: 250; margin: 50px 0px 0px 20px;float:left '>";
    

        $body .= "<div id='scoreTableHeaderDiv' class='' style='width:400px;margin:0px 0px 0px 0px '>
                <table class='whoopieScoringTable tr1 tr2 tr3 tr4 tr5 tc6 tr7 tr8 tr9 tr10' style='border: 2px solid black;'>
                <thead>
                <tr>         <th style='border-right: 2px solid gray; text-align:center;width:35px'>Hand</th>
                             <th style='background-color: lightblue; text-align:center' colspan=4>Bids</th>
                             <th style='background-color: black;width:10px; '>&nbsp</th>
                             <th style='background-color: lightblue; text-align:center'colspan=4>Scores</th></tr>
                <tr>   
                    <th   valign=top style='border-right: 2px solid gray;width:35px'>No.</th>
                    <th   valign=top style='width:30px'>G</th>
                    <th   valign=top style='width:30px'>J</th>
                    <th   valign=top style='width:30px'>S</th>
                    <th   valign=top style='width:30px'>E</th>
                    <th style='background-color: black;width:10px'>&nbsp</th>
                
                
                    <th   valign=top style='width:30px'>G</th>
                    <th   valign=top style='width:30px'>J</th>
                    <th   valign=top style='width:30px'>S</th>
                    <th   valign=top style='width:30px'>E</th>
                 </tr></thead></table></div>";


        $body .= "<div id='scoreTableDiv' class='' style=' width:400px;margin:0px 0px 0px 0px '>";
        $body .= "<table id='scoreTable' class='whoopieScoringTable tr1 tr2 tr3 tr4 tr5 tc6 tr7 tr8 tr9 tr10' style='border: 0px solid blue;'>";

        $body .= "  <thead>
                    <tr>
                        <th valign=top style='border-right: 2px solid gray;width:35px'></th>
                        <th valign=top style='width:30px'></th>
                        <th valign=top style='width:30px'></th>
                        <th valign=top style='width:30px'></th>
                        <th valign=top style='width:30px'></th>
                        <th style='background-color: black;width:10px'></th>
                        <th valign=top style='width:30px'></th>
                        <th valign=top style='width:30px'></th>
                        <th valign=top style='width:30px'></th>
                        <th valign=top style='width:30px'></th>
                    </tr></thead><tbody></tbody></table></div>";

        $body .= "<div id='scoreTableTotalsDiv' class='' style=' width:400px;margin:0px 0px 0px 0px '>";
        $body .= "<table id='scoreTableTotals' class='whoopieScoringTable tr1 tr2 tr3 tr4 tr5 tc6 tr7 tr8 tr9 tr10' style='border: 2px solid black;'>";

        $body .= "  <tbody>
                    <tr>
                        <td valign=top style='width:37px'></th>
                        <td valign=top style='width:30px'></th>
                        <td valign=top style='width:30px'></th>
                        <td valign=top style='width:30px'></th>
                        <td valign=top style='width:30px'></th>
                        <td style='background-color: black;width:10px'></th>
                        <td id='totalsID0' valign=top style='width:30px;color:blue'></th>
                        <td id='totalsID1' valign=top style='width:30px;color:blue'></th>
                        <td id='totalsID2' valign=top style='width:30px;color:blue'></th>
                        <td id='totalsID3' valign=top style='width:30px;color:blue'></th>
                    </tr></tbody></table></div>";

        $body .= "</div>";  // whoopie score container

        $body .= "<div id='whoopieMessageDiv2' style='clear:both; display:none; background-color: white; margin: 10px 0px 0px 20px;
                                                      width:700px; height: 80px;'>
        <div id='whoopieMessageDiv' style='color:black;   margin: 0px 0px 0px 275px;font-size:28px'><span id='whoopieMessage'></span>";
        

        $body .= "</div></div>";



        return($body);
    }


    function getConvertTotal($mod) {
        $convList = array();

        $totalInvest = 0;
        $this->getConvertsforModel($mod, $convList);
        for ($i=0; $i < $mod->numberConverts; $i++ ) {
            $totalInvest += $convList[$i]->totalInvestment;
        }
        return($totalInvest);

    }

 /*
  * return all the converts for a model via the passed by reference array
  */
    function  getConvertsforModel($mod, &$convList) {

        $rows = $mod->getRows('converttable', "model_id=$mod->id");
        $i = 0;
        while($conv = mysqli_fetch_object($rows,"angelConvert")){
            $convList[$i] = $conv;
            $i++;
        }
    }
/*****************************************************************************
 * @param $investorID  -- logged in user id of investor
 * @param $investID -- if this is a conversion, id of investment to be converted (or 0) -- i am pretty sure this is deprecated!
 * @param $modelID -- shared model to display or 0.
 * @param $hash --  if model != 0, this must match the hash in the model
 * @param $action -- gives path back - either investments or model (the latter if modeling, the former if converting)
 * @return string
 */

    function convertPageBody($investorID, $investID, $modelID, $action, $message) {

        $initAction = "default";
        $path = $GLOBALS["path"];

        $page = "";

        $page .= $this->convertPageOutput();    // this is the div containing the result tables

        $page .=  "<div style='float:left; margin: 0px 0px 0px 20px'><form id='modelChooserForm' action='$path/$action' method='post' enctype='multipart/form-data'>";
        $mod = new angelModel();
        $mod->id = $modelID;
        $conn = $mod->connect();
        /*
         * If a modelID is passed in we should start with that model as it was just saved or is about to be edited
         */
        if ($modelID > 0) {
            $modelInput = $mod->getEntry();    // get entry of the input model

            /*
             * arrange for the model to be loaded upon entry -- none of this is used now (delete soon)
             */
            if ($message != "")
                $x = "loadModelByID($modelID,'','')" . ";$('#saveButton').notify('$message', {position:'right bottom', className:'success'})";

            $initAction = "loadModel";
        } else {
            if ($investID != 0) {
                /* we are converting a note/safe */
                $x = "document.getElementById('convertTable').style.display = 'block';";
                $initAction = "converting";
            }
            $modelInput = $mod;                                         // just a blank model
        }


        $modelRows = 0;
        if ($investorID != $GLOBALS["guestID"])
            $modelRows = $mod->getRows("modeltable", "investor_id=$investorID"); // get this user's models, if any


        if ($investID == 0) {
            /* we are modeling only */
            $inv = new angelInvest();
            $inv->type = "safe";    // assume a SAFE
            $invAmount = $mod->yourInvestment;
            $cap = 0;
            $illegalEntryText = "Please enter a number.";
            // $page .= "<span style='font-size:20px; font-weight: bold '>Conversion Modeling</span>";
            $submitActionKey = "saveModel";
        }

        /*
         * assuming nothing bad happens, I think this code is no longer used
        else {
            $submitActionKey = "saveConvert";
            $inv = new angelInvest();
            $inv->id = $investID;
            $inv = $inv->getEntry();
            $invAmount = $inv->invest_amount;
            $cap = $inv->cap;

            $co =  new angelCompany();
            $co->id = $inv->company_id;
            $conn = $co->connect();
            $coName = $co->companyName();

            $illegalEntryText = "Please enter a number.";
            $page .= "<span style='font-size:20px; font-weight: bold '>Conversion for $coName $inv->type.</span>";
        }
        */


        $modelName = $mod->name;          // name of input model or blank if none
        $modelID = $mod->id;

        $page .= "</form></div>";


        /*
         * If the modelID is not 0 (new model), then allow the user to edit the name
         */
        if ($modelID != 0 and $modelInput->name != "") {
            $page .= "<div id='modelNameDiv' style='float:left; margin: 0px 0px 0px 0px; display:inline;'>";
            $page .= "You are editing model: &nbsp <span id='modelNameForm' style='color:blue;font-size:20px;' contentEditable='true'>$modelInput->name</span>   &nbsp &nbsp(to change the name edit here and hit Save below)";
            $page .= "</div>";

        } else {
            $page .= "<input type='hidden' id='modelNameForm' value=''>";    // make sure the id exists so code that assumes it is there doesn't break
        }
        $page .= "<div id='modelShareDiv' style='float:right; margin: 20px 60px 6px 0px; display:none;'>
                            <div style='float:right; width:600px; border: 0px solid red;'>
                                <div style='float: left; padding:10px;'>Copy this URL to share this model:</div>
                                <input id='foo' value=''>
                                <button class='clipButton' id='clipButton' data-clipboard-target='#foo'>
                                    <img src='img/clippy.svg' width='15' height='15' alt='Copy to clipboard' title='Copy to clipboard'>
                                </button>
                            </div>

                      </div>";

        $page .= "<div style='display:inline-block; margin: 20px 0px 0px 0px'></div>";
        $page .= "<div style='clear:both; background-color: #E0E0E0; margin: 30px 0px 0px 20px; padding:10px 30px 10px 30px;width:90%;'>

                            <a href='javascript:onclick=saveModel(\"\")' class='modelButton' id='saveButton'>Save</a>
                            <span style='margin: 0px 0px 0px 30px;color:white'>
                            <a href='javascript:onclick=modelInitialCap()' class='modelButton'>Show Initial Cap Table</a>
                            <a class='modelButton' href='javascript:onclick=modelConvertOnly()'>Show Cap Table pre-Equity</a>
                            <a href='javascript:onclick=modelEquity()' class='modelButton'>Show Cap Table post-Equity</a>

                            </span>
                            <a title='What do these buttons do?'
                                onClick=window.open('/static/modelbuttonhelp.html','CO','width=700,height=700,0,status=0,location=0,');
                                href='javascript:void(0);'><big><strong>?</strong></big></a>



                            ";

        $page .=  "<div style='float:right; clear:both; margin: 20px 0px 0px 25px'>Load Model:
                  <input type='hidden' name='model'>
                  <select id='saveModelSel' name='savedModel' onchange=loadModel(this)><option value=0>New Model</option>";

        if ($modelRows) {
            while($m = mysqli_fetch_object($modelRows,"angelModel")) {    // retrieve all the saved models for this user
                if ($m->id == $mod->id)
                    $selected = "selected";
                else
                    $selected = "";
                $page .= "<option value=$m->id $selected>$m->name</option>";                 // and allow them to be selected
            }
            $page .= "</select></div></div>";
        } else {
            /*
             *  if modelID is -1, this is a request to load the sample model
             */
            if ($modelID == -1) {
                $page .= "<option value=-1 selected>Sample Model</option>";
                $initAction = "loadSample";
                $page .= "</select> Sample Model: Click a button to the left to see the cap table or on ";
                $page .= "<a href='javascript:onclick=document.getElementById(\"saveModelSel\").value=0;clearModel(\"\")'>Clear</a> to start your own.";
                $page .= "</div>";
            } else {
                $page .= "</select></div></div>";
            }
        }
        $page .= "<script>$(document).ready( function() { initializeModel('$initAction', $investorID, $modelID, '$message') } );</script>";  // initialize things when the page loads


        /*
         * the input portion of the page
         */

        $page .= $this->modelInputForm("");




        return($page);

    }       /* convert */

    /*****************************************************************************
     * @param $investorID  -- logged in user id of investor
     * @param $investID -- if this is a conversion, id of investment to be converted (or 0) -- i am pretty sure this is deprecated!
     * @param $modelID -- shared model to display or 0.
     * @param $hash --  if model != 0, this must match the hash in the model
     * @param $action -- gives path back - either investments or model (the latter if modeling, the former if converting)
     * @return string
     */

    function convertPageBody2($investorID, $investID, $modelID, $action, $message) {

        $initAction = "default";
        $path = $GLOBALS["path"];

        $page = "";

        //$page .= $this->createLeftNav($investorID);
        $page .= $this->convertPageOutputTest();    // this is the div containing the result tables

        $page .=  "<div style='float:left; margin: 0px 0px 0px 20px'><form id='modelChooserForm' action='$path/$action' method='post' enctype='multipart/form-data'>";
        $mod = new angelModel();
        $mod->id = $modelID;
        $conn = $mod->connect();
        /*
         * If a modelID is passed in we should start with that model as it was just saved or is about to be edited
         */
        if ($modelID > 0) {
            $modelInput = $mod->getEntry();    // get entry of the input model

            /*
             * arrange for the model to be loaded upon entry -- none of this is used now (delete soon)
             */
            if ($message != "")
                $x = "loadModelByID($modelID,'','')" . ";$('#saveButton').notify('$message', {position:'right bottom', className:'success'})";

            $initAction = "loadModel";
        } else {
            if ($investID != 0) {
                /* we are converting a note/safe */
                $x = "document.getElementById('convertTable').style.display = 'block';";
                $initAction = "converting";
            }
            $modelInput = $mod;                                         // just a blank model
        }


        $modelRows = 0;
        if ($investorID != $GLOBALS["guestID"])
            $modelRows = $mod->getRows("modeltable", "investor_id=$investorID"); // get this user's models, if any


        if ($investID == 0) {
            /* we are modeling only */
            $inv = new angelInvest();
            $inv->type = "safe";    // assume a SAFE
            $invAmount = $mod->yourInvestment;
            $cap = 0;
            $illegalEntryText = "Please enter a number.";
            // $page .= "<span style='font-size:20px; font-weight: bold '>Conversion Modeling</span>";
            $submitActionKey = "saveModel";
        } else {
            $submitActionKey = "saveConvert";
            $inv = new angelInvest();
            $inv->id = $investID;
            $inv = $inv->getEntry();
            $invAmount = $inv->invest_amount;
            $cap = $inv->cap;

            $co =  new angelCompany();
            $co->id = $inv->company_id;
            $conn = $co->connect();
            $coName = $co->companyName();

            $illegalEntryText = "Please enter a number.";
            $page .= "<span style='font-size:20px; font-weight: bold '>Conversion for $coName $inv->type.</span>";
        }



        $modelName = $mod->name;          // name of input model or blank if none
        $modelID = $mod->id;

        $page .= "</form></div>";

        $page .= "<script>$(document).ready( function() { initializeModel('$initAction', $investorID, $modelID, '$message') } );</script>";  // initialize things when the page loads

        $page .= "<div id='modelShareDiv' style='float:right; margin: 20px 60px 6px 0px; display:none;'>
                            <div style='float:right; width:600px; border: 0px solid red;'>
                                <div style='float: left; padding:10px;'>Copy this URL to share this model:</div>
                                <input id='foo' value=''>
                                <button class='clipButton' id='clipButton' data-clipboard-target='#foo'>
                                    <img src='img/clippy.svg' width='15' height='15' alt='Copy to clipboard' title='Copy to clipboard'>
                                </button>
                            </div>

                      </div>";

        $page .= "<div style='display:inline-block; margin: 20px 0px 0px 0px'></div>";
 


        /*
         * the input portion of the page
         */

        $page .= $this->modelInputFormTest("");




        return($page);

    }       /* convert2 */

    function convertPageInput($action) {

        $path = $GLOBALS["path"];

        $submitActionKey = "saveModel";

        /*
         * the input portion of the page
         */

        $page = $this->convertDialogs();

        $page .= "  <div style='clear:both'></div>
                    <div style=' border: 0px solid blue; background-color: #E0E0E0; margin:20px 0px 20px 20px; padding: 10px 30px 10px 30px; width:90%;'>

                        <form id='convertForm' action='$path/$action' method='post' enctype='multipart/form-data'>

                        <input type='hidden' name='investID' value='0'>
                        <input type='hidden' name=$submitActionKey>";

        /*
         * founders
         */
        $page .= "
                           <span class='modelLabel'>Founders -- <a href='javascript:onclick=addFounder()'>Add Founder</a></span>
                            <p></p>
                            <script type='text/javascript'>$(function() {
                                        $('#founderSharesForm').number(true);
                                });</script>
                            <table id='founderTable' class='convertInputTable'   style='display: none'>
                                    <th style='width:275px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:150px'>Shares of Founder (Common) Stock</th>
                                <tr>

                                </tr><tbody></tbody></table><br />";

        /*
         * convertibles
         */
        $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
        $page .= "
                           <span class='modelLabel'>Convertibles -- <a href='javascript:onclick=addConvert()'>Add Convert</a></span>

                            <p></p>
                            <table id='convertTable' class='convertInputTable'   style='display: none; width:95%'>

                                <tr><th style='width:160px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:150px'>Your Investment, w/Interest</th>
                                    <th style='width:100px'>Total Invested in this Convertible w/Interest</th>
                                    <th style='width:100px'>Discount (%)</th>
                                    <th style='width:100px'>Cap</th>
                                    <th style='width:200px'>Special</th>

                                </tr><tbody></tbody></table><br />";
        /*
          * equity
          */
        $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
        $page .= "
                                <span class='modelLabel'>Equity -- <a href='javascript:onclick=addEquityRound()'>Add Round</a></span>
                            <p></p>
                            <table class='equityInputTable' id='equityTable' style='border-top: 0px solid black; display:none;width:95%'>
                            <tr>
                                <th style='width:160px' >Round</th>
                                <th style='width:150px'>Your Investment (If any)</th>
                                <th style='width:100px'>Total Investment</th>
                                <th style='width:130px'>Post-Money Option Pool(%)</th>

                                <th style='width:100px'>Valuation</th>
                                <th style='width:140px'>Fully Diluted Pre-Money Shares</th>
                                <th style='width:140px'>Unallocated Pre-Money Options</th>

                            </tr>
                            <tr><td style='width:100px; '>Series A</td>
                                <script type='text/javascript'>$(function() {
                                        $('#yourAmount').number(true);
                                        $('#investAmount').number(true);
                                        $('#valuation').number(true);
                                        $('#sharesOut').number(true);
                                        $('#opoolPre').number(true);
                                });</script>
                                <td><input size='10%' type=text id='yourAmount'     name='yourAmount' value='' /></td>
                                <td><input size='10%' type=text id='investAmount'   name='investAmount' value=''/></td>
                                <td style='width:40px; border-spacing: 0px; padding: 0px '>
                                    <input size=4 type=text     id='opoolPost'      name='opoolPost' value='' /></td>
                               <td><input size='10%' type=text id='valuation'       name='valuation' value='' /><br />
                                      <input type='radio'       id='valispre'       name='pre' value='pre' checked> Pre
                                       <input type='radio'                          name='pre' value='post'> Post</td>
                                <td><input size='10%' type=text id='sharesOut'      name='sharesOut' value='' /></td>
                                <td><input size='10%' type=text id='opoolPre'       name='opoolPre' value=''/></td>
                            </tr>
                            </table><br />
                            ";

        $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
        $page .= "<input type='hidden' id='convertCount' name='convertCount' value='0'>";
        $page .= "<input type='hidden' id='founderCount' name='founderCount' value='0'>";

        $page .= "
                            <input type='hidden' name='yourPriceForm' value='0' id='yourPriceForm'>
                            <input type='hidden' name='yourSharesForm' value='0' id='yourSharesForm'>
                            <input type='hidden' name='yourOwnershipForm' value='0' id='yourOwnershipForm'>
                            <input type='hidden' name='postFDSharesForm' value='0' id='postFDSharesForm'>
                            <input type='hidden' name='postPriceForm' value='0' id='postPriceForm'>
                            <input type='hidden' name='modelName' value='' id='modelName'>
                            <input type='hidden' name='modelID' value='0' id='modelID'>

                            <a href='javascript:onclick=modelEquity()' class='modelButton'>Model</a>
                            <a href='javascript:onclick=saveModel(\"\")' class='modelButton' id='saveButton'>Save</a>

                            <div style='float:right'>Precision for calculations:
                            <select id='precision' name='precision'>
                                <option value='-1' selected>Maximum</option>
                                <option value='4'>4 decimal places</option>
                                <option value='3'>3 decimal places</option>
                                <option value='2'>2 decimal places</option>
                            </select></div>



                        </form>
                    </div>";



        if (angelIsLoggedIn()) {
            $page .= '<p></p><H3>&nbsp &nbsp Instructions (<a href="javascript:onclick=toggleInstructions()"><span id="showHideInstructions">show</span></a>)</H3>';
            $page .= '<div id="modelInstructions" style="display:none" class="modelInstructions">';
        } else {
            $page .= '<p></p><H3>Instructions (<a href="javascript:onclick=toggleInstructions()"><span id="showHideInstructions">hide</span></a>)</H3>';
            $page .= '<div id="modelInstructions" class="modelInstructions">';
        }
        $page .= file_get_contents("includes/helpInstructions.inc");        // add instructions at the end of the page
        $page .= "</div>";


        return($page);

    }       /* convertPageInput */

    function convertDialogs() {

        $path = $GLOBALS["path"];
        $page = "";

        $page .= "<script type='text/javascript'>$(function() {
                $('#founderShares').number(true);
                $('#dialog-fullyDilutedShares').number(true);
                $('#optionPoolSize').number(true);
                $('#grantedOptions').number(true);
                $('#promisedOptions').number(true);
                $('#unissuedOptions').number(true);
                });</script>";

        $page .= "
                <div id='dialogAddFounder' title='Add Founder' style='display:none'>
                           <table>
                           <tr><td>Founder's Name:</td><td><input type='text' id='founderName'></td></tr>
                           <tr><td>Founder's Shares:</td><td><input type='text' id='founderShares'></td></tr>
                        </table>
                </div>
                <div id='dialogDeleteFounder' title='Delete Founder' style='display:none'>
                         Delete the founder named <span id='founderToDelete'></span>?
                </div>
                <div id='dialogDeleteCommonHolder' title='Delete Common Holder' style='display:none'>
                         Delete the common stock holder named <span id='commonHolderToDelete'></span>?
                </div>
                <div id='dialogAddCommon' title='Add Common Stock Holder' style='display:none'>

                           Choose common holder: <select onchange='addCommonSelector(this)'  id='newCommonType' name='newCommonType'>
                                                        <option style='color:green' value='' selected disabled>Make your choice</option>
                                                        <option value='Founder'>Founder</option>
                                                        <option value='YC'>YC</option>
                                                        <option value='Options'>Options</option>
                                                        <option value='Other'>Other</option>
                                                    </select>
                     <div id='addCommonDiv'  style='margin: 10px 0px 0px 30px; display:none'>
                       <table>
                           <tr><td>Name:</td><td><input type='text' id='commonHolderName'></td></tr>
                           <tr><td>Shares:</td><td><input type='text' id='commonHolderShares'></td></tr>
                        </table>
                     </div>
                     <div id='addYCDiv'  style='margin: 10px 0px 0px 30px; display:none'>
                       <table>
                           <tr><td>YC's Shares:</td><td><input type='text' id='YCCommonShares'></td></tr>
                        </table>
                     </div>
                     <div id='addOptionsDiv'  style='margin: 10px 0px 0px 30px; display:none'>

                     <a title='What Are These?' onClick=window.open('/static/optionshelp.html','CO','width=700,height=350,0,status=0,location=no,toolbar=no,menubar=no'); href='javascript:void(0);'>
                     <strong>Options Help</strong></a><p></p>

                       <table>
                           <tr><td>Total Option Pool:</td><td><input onchange='modelOptionPool(\"\");' type='text' id='optionPoolSize'></td></tr>
                           <tr><td>Granted Options:</td><td><input onchange='modelOptionPool(\"\");' type='text' id='grantedOptions'></td></tr>
                           <tr><td>Promised Options:</td><td><input onchange='modelOptionPool(\"\");' type='text' id='promisedOptions'></td></tr>
                           <tr><td>Unissued Options:</td><td><input onchange='modelOptionPool(\"\");' type='text' id='unissuedOptions'></td></tr>
                        </table>
                        <p />
                        <span>The Option Pool is the sum of Granted, Promised, and Unissued Options.</span>
                     </div>

                </div>
                <div id='dialogAddConvert' title='Add Convertible' style='display:none'>

                           Choose your convertible: <select id='newConvertType' name='newConvertType'>

                                                        <option value='Post SAFE' selected>Standard Post-Money Safe</option>
                                                        <option value='SAFE'>Standard Pre-Money Safe</option>
                                                        <option value='Note'>Standard Note</option>

                                                        <option value='Custom'>Custom Convertible</option>
                                                        <option value='Manual'>Manual Convertible</option>

                                                        <option value='YCVC SAFE'>(YC Companies only)YCVC Safe (through S17)</option>
                                                        <option value='YCII SAFE'>(YC Companies only)YCII Safe (W18 and S18)</option>
                                                        <option value='YC POST'>(YC Companies only)YCII/III Post Safe (W19 +)</option>
                                                    </select>

                </div>
                <div id='dialogGetFDShares' title='Enter FD Shares' style='display:none'>

                           Please enter the total number of fully diluted shares in the company: <p></p>
                                    <input type='text' id='dialog-fullyDilutedShares'>

                </div>
                <div id='dialogDelete' title='Delete Convertible' style='display:none'>

                           Delete this convertible?<br />
                             Note that convertibles will be renumbered and any preexisting lists will need
                             to be manually corrected.

                </div>
                <div id='dialogPMSafe' title='Post Money Safe' style='display:none'>


                        <table class='convertCustomTable'>

                         <tr><td> Include in Denominator</td>
                              <td>
                                 <input type='checkbox' id='pmsafeDenomUnissued' name='pmsafeDenom' value='Y' checked> Unissued Options prior to round<br />
                                 <input type='checkbox' id='pmsafeDenomAdditional' name='pmsafeDenom' value='Y'> Additional Options created as part of round<br />

                              </td></tr>
                        </table>
                        <p></p>
                        Note: YC Standard Post-Money safes include unissued options but NOT additional options.


                </div>
                <div id='dialogCustom' title='Custom Convertible' style='display:none'>


                        <table class='convertCustomTable'>

                        <tr><td>Use Cap when:</td><td><input type='radio' id='customCompareCapAndVal' name='customCompare' value='Y' checked>Cap < Valuation<br>
                                         <input type='radio' id='customCompareCapPrice' name='customCompare' value='Y'>Cap price < (Discounted)Equity price </td></tr>

                         <tr><td> Include in Denominator</td>
                              <td>
                                 <input type='checkbox' id='customDenomIssued' name='customDenom' value='Y' checked> Issued Shares<br />
                                 <input type='checkbox' id='customDenomUnissued' name='customDenom' value='Y' checked> Unissued Options prior to round<br />
                                 <input type='checkbox' id='customDenomAdditional' name='customDenom' value='Y' checked> Additional Options created as part of round<br />
                                 <input type='checkbox' id='customDenomThis' name='customDenom' value='Y' >This convertible<br />
                                 <input type='checkbox' id='customDenomPreexist' name='customDenom' value='Y' >Pre-existing Convertibles (to be specified)<br />

                              </td></tr>
                        </table>


                </div>";

        return($page);

    }       // convertDialogs()

    function convertPageOutput() {


        /*
         * Summary capitalization
         */
        $page = '
           <div id="outputDiv" style="padding: 10px 0px 10px 30px; margin: 0px 0px 0px 20px ;float:left;border: 6px solid #d3d3d3; width:93%;  display: none">';

        $page .= "<div title='print!!' style='float:right; margin:0px 30px 0px 0px;'><a href='javascript:onclick=angelPrint(\"convertSummary\")'><img src='img/printer_icon.png' height='20px' width='20px'></a></div>";

        $page .= '<div id="financingSummary" style="float: left; margin: 30px 60px 0px 20px; display: none;">';

        $page .= '
                      <table class="angelConvertSummaryTable" style="font-size: 14px; ">
                        <tr><td colspan=2 id="financingSummaryTitle" style="color: #1574B1; font-size: 16px;">Post Financing Summary</td></tr>

                        <tr><td></td><td>Valuation:</td>                    <td id="postVal"></td></tr>
                        <tr><td></td><td>Total FD Shares:</td>              <td id="postFDShares"></td></tr>
                        <tr id="preFDSharesRow" style="display:none" ><td></td><td>Pre-Money FD Shares:</td>
                                                                            <td id="preFDShares"></td></tr>
                        <tr id="newSharesRow" style="display:none"><td></td><td>New Shares:</td>
                                                                            <td id="newShares"></td></tr>
                        <tr><td></td><td>Available Options:</td>            <td id="postOptions"> </td></tr>
                        <tr><td></td><td>Share Price:</td>                  <td id="postPrice"></td></tr>
                        <tr><td colspan=3><hr></td></tr>
                        <tr><td></td><td>VC Ownership:</td>                 <td id="postVCOwnership"></td></tr>
                        <tr><td></td><td>Convert Ownership:</td>            <td id="postConvOwnership"></td></tr>
                        <tr><td></td><td>Founder Ownership:</td>            <td id="postFounderOwnership"></td></tr>
                        <tr><td></td><td>Other Common Ownership:</td>       <td id="postCommonOwnership"></td></tr>
                        <tr><td></td><td>Options:</td>                      <td id="postOptionsPercent"></td></tr>
                        <tr><td colspan=3><hr></td></tr>
                        <tr><td></td><td>Total:</td>                        <td id="postOwnershipTotal"></td></tr>

                     </table>
               </div>';


        $page .= '<div id="financingDetails" style="float: left; margin: 30px 0px 0px 20px; display: none;">';
        $page .= '<span id="capTableTitle" style="color: #1574B1; font-size: 16px; ">Post Financing Cap Table</span><br />';
        $page .= '   <table class="angelCapTable" id="capTable">

                        <tr> <th style=" width:200px; background-color: #F6F6EA "></th>
                             <th style=" width:125px">Price/Share</th>
                             <th style=" width:125px">Shares</th>
                             <th style=" width:125px">%</th>
                             <th style=" background-color: #F6F6EA; border: 0px solid black; width:125px;"></th>
                        </tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Common</td>
                            <td id="commonPrice" style="font-weight: bold"></td>
                            <td id="commonShares" style="font-weight: bold"></td>
                            <td id="commonOwnership" style="font-weight: bold"></td>

                        </tr>';

        $page .= '
                        <tr><td  style="border-right: 0px;" colspan="5"><table class="angelCapTable" id="founderOwnershipTable"><thead>
                        <tr><th style="color: #1574B1; width:200px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px; text-align:right; width:100px; border: 0px;"></th>
                        </tr></thead><tbody></tbody></table>
                        </td></tr>';
        $page .= '
                        <tr><td  style="border-right: 0px;" colspan="5"><table class="angelCapTable" id="otherCommonOwnershipTable"><thead>
                        <tr><th style="color: #1574B1; width:200px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px; text-align:right; width:100px; border: 0px;"></th>
                        </tr></thead><tbody><tr><td>1</td><td>2</td><td>3</td><td>4</td></tr></tbody></table>
                        </td></tr>';

        $page .= '      <tr><td style="text-align: right">Other Common</td>
                            <td id="otherCommonPrice"></td>
                            <td id="otherCommonShares"></td>
                            <td id="otherCommonOwnership"></td>

                        </tr>';

        $page .= '      <tr><td colspan="4"><hr></td></tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Preferred</td>
                            <td id="preferredPrice" style="font-weight: bold"></td>
                            <td id="preferredShares" style="font-weight: bold"></td>
                            <td id="preferredOwnership" style="font-weight: bold"></td>

                        </tr>';

        $page .= '      <tr><td style="text-align: right"><span title="Show Price Calc" ><a onClick="displayEquation(this,\'vcEquationDiv\')">VC</a></span></td>
                            <td id="vcPrice"></td>
                            <td id="vcShares"></td>
                            <td id="vcOwnership"></td>
                        </tr>';

        $page .= '      <tr><td style="border-right: 0px;" colspan="5">
                            <table class="angelCapTable" id="convertOwnershipTable"><thead>
                            <tr><th style="text-align:right; color: #1574B1; width:200px"></td>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px; text-align:right; border: 0px;"></th>
                             </tr></thead><tbody></tbody></table>
                         </td></tr>';



        $page .= '      <tr><td colspan="4"><hr></td></tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Options</td>
                            <td style="font-weight: bold"></td>
                            <td id="optionsShares" style="font-weight: bold"></td>
                            <td id="optionsOwnership" style="font-weight: bold"></td>
                        </tr>
                        <tr><td style="color: #1574B1; text-align: right">Granted</td>
                            <td></td>
                            <td id="grantedOptionsShares"></td>
                            <td id="grantedOptionsOwnership"></td>
                        </tr>
                        <tr><td style="color: #1574B1; text-align: right">Promised</td>
                            <td></td>
                            <td id="promisedOptionsShares"></td>
                            <td id="promisedOptionsOwnership"></td>
                        </tr>
                        <tr><td style="color: #1574B1; text-align: right">Available</td>
                            <td></td>
                            <td id="availableOptionsShares"></td>
                            <td id="availableOptionsOwnership"></td>
                        </tr>';

        $page .= '      <tr><td colspan="4"><hr></td></tr>';
        $page .= '      <tr><td colspan="4"><hr></td></tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Your Equity</td>
                            <td id="yourPrice" style="font-weight: bold"></td>
                            <td id="yourShares" style="font-weight: bold"></td>
                            <td id="yourOwnership" style="font-weight: bold"></td>

                        </tr>';

        $page .= '      <tr><td  style="border-right: 0px;" colspan="5">
                            <table class="angelCapTable" id="ownershipTable"><thead>
                            <tr><th style="color: #1574B1; width:200px; text-align: left"></th>
                                    <th style="width:125px" style="font-weight: bold"></th>
                                    <th style="width:125px" style="font-weight: bold"></th>
                                    <th style="width:125px" style="font-weight: bold"></th>
                                    <th style="width:125px; text-align:right; border: 0px;"></th>
                             </tr></thead><tbody></tbody></table>
                         </td></tr>

                         ';

        $page .= '     </table>';   // captable
        $page .= '<p></p><span style="text-align: left; color: #0000BB">You may click on each entry
                                  in the Preferred section above to show how each price per share
                                   was calculated.</span>';
        $page .= '</div>';          // financing Details

        $page .= '<div id="equationDiv" style="float: left; margin: 30px 0px 0px 20px; display: block;">';

        $page .=  '
                       <table id="equationTable">
                         <tr><td>
                           <div class="angelEquationDiv" id="vcEquationDiv" style="display:none">
                            <span style="text-align: left; color: #1574B1">VC Calculation</span>
                            <table class="angelEquationTable" style="width: 300px;" >

                                <tr><td style="width:50px"></td>  <td style="width:200px;" id="vcEquationNum">Numerator</td></tr>

                                <tr><td id="vcPriceEquals" style="text-align: right; padding-right: 10px; width: 50px;">price =</td>
                                    <td><hr></td></tr>
                                <tr><td></td><td style="width:200px" id="vcEquationDenom">Denominator</td></tr>

                             </table></div>
                         </td></tr>
                         <tr><td>
                           <div class="angelEquationDiv" id="angelEquationDiv" style="display:none">
                            <table class="angelEquationTable" width="300px">
                                <tr><td colspan="2" style="text-align: left; color: #1574B1">Angel Calculation</td></tr>

                                <tr><td></td><td id="your0EquationNum">Numerator</td></tr>
                                <tr><td id="angelPriceEquals" style="text-align: right; padding-right: 10px">price =</td><td><hr></td></tr>
                                <tr><td></td><td  width="400px" id="your0EquationDenom">Denominator</td></tr>
                            </table></div>
                         </td></tr>

                       </table>';

        $page .= '</div>';          // equation div.
        $page .= '</div>';          // output div.
        $page .= '<div style="clear:both"></div>';

        return($page);
    }   // convertpageoutput()

    function convertPageOutputTest() {

        /*
         * Summary capitalization
         */
        $page = '
           <div id="outputDiv2" class="angelConvertDiv" style="position: fixed; display: inline">';

        $page .= "<div title='print!!' style='float:right; margin:0px 30px 0px 0px;'><a href='javascript:onclick=angelPrint(\"convertSummary\")'><img src='img/printer_icon.png' height='20px' width='20px'></a></div>";

        $page .= '<div id="" style="float: left; margin: 30px 60px 0px 20px; display: inline;">';
        $page .= "<div style='clear:both; margin: 5px 0px 5px 0px'><a href='javascript:onclick=modelEquity()' class='modelButton'>Model</a><p />
                            <a href='javascript:onclick=saveModel(\"\")' class='modelButton' id='saveButton'>Save</a><p />
                            <a href='javascript:onclick=loadModel(\"\")' class='modelButton' id='loadButton'>Load</a><p />
                          <span style='margin: 0px 0px 0px 0px;color:white'><a class='modelButton' href='javascript:onclick=modelConvertOnly()'>Model converts</a></span>
                            <a title='What does this do?' onClick=window.open('/static/convertonlyhelp.html','CO','width=700,height=300,0,status=0,'); href='javascript:void(0);'>?</a>
                                </div>
                            ";
        $page .= "<div style='float:right'>Precision:
            <select id='precision' name='precision'>
                                    <option value='-1' selected>Max</option>
                                    <option value='4'>.0001</option>
                                    <option value='3'>.001</option>
                                    <option value='2'>.01</option>
                                </select></div>";
        $page .= '</div>';

        $page .= '<div id="financingSummary" style="float: left; margin: 30px 60px 0px 20px; display: inline;">';

        $page .= '
                      <table class="angelConvertSummaryTable" style="font-size: 14px; ">
                        <tr><td colspan=2 style="color: #1574B1; font-size: 16px;">Post Financing Summary</td></tr>

                        <tr><td></td><td>Valuation:</td>                    <td id="postVal"></td></tr>
                        <tr><td></td><td>Total FD Shares:</td>              <td id="postFDShares"></td></tr>
                        <tr><td></td><td>Options:</td>                      <td id="postOptions"> </td></tr>
                        <tr><td></td><td>Share Price:</td>                  <td id="postPrice"></td></tr>
                        <tr><td colspan=3><hr></td></tr>
                        <tr><td></td><td>VC Ownership:</td>                 <td id="postVCOwnership"></td></tr>
                        <tr><td></td><td>Convert Ownership:</td>            <td id="postConvOwnership"></td></tr>
                        <tr><td></td><td>Founder Ownership:</td>            <td id="postFounderOwnership"></td></tr>
                        <tr><td></td><td>Other Common Ownership:</td>            <td id="postCommonOwnership"></td></tr>
                        <tr><td></td><td>Options:</td>                      <td id="postOptionsPercent"></td></tr>
                         <tr><td colspan=3><hr></td></tr>
                        <tr><td></td><td>Total:</td>                        <td id="postOwnershipTotal"></td></tr>

                     </table>
               </div>';


        $page .= '<div id="financingDetails" style="float: left; margin: 30px 0px 0px 20px; display: inline;">';
        $page .= '<span style="color: #1574B1; font-size: 16px; ">Post Financing Cap Table</span><br />';
        $page .= '   <table class="angelCapTable" id="capTable">

                        <tr> <th style=" width:200px; background-color: #F6F6EA "></th>
                             <th style=" width:125px">Price/Share</th>
                             <th style=" width:125px">Shares</th>
                             <th style=" width:125px">%</th>
                             <th style=" background-color: #F6F6EA; border: 0px solid black; width:125px;"></th>
                        </tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Common</td>
                            <td id="commonPrice" style="font-weight: bold"></td>
                            <td id="commonShares" style="font-weight: bold"></td>
                            <td id="commonOwnership" style="font-weight: bold"></td>

                        </tr>';

        $page .= '
                        <tr><td  style="border-right: 0px;" colspan="5"><table class="angelCapTable" id="founderOwnershipTable"><thead>
                        <tr><th style="color: #1574B1; width:200px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px; text-align:right; width:100px; border: 0px;"></th>
                        </tr></thead><tbody></tbody></table>
                        </td></tr>';
        $page .= '
                        <tr><td  style="border-right: 0px;" colspan="5"><table class="angelCapTable" id="otherCommonOwnershipTable"><thead>
                        <tr><th style="color: #1574B1; width:200px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px; text-align:right; width:100px; border: 0px;"></th>
                        </tr></thead><tbody><tr><td>1</td><td>2</td><td>3</td><td>4</td></tr></tbody></table>
                        </td></tr>';

        $page .= '      <tr><td style="text-align: right">Other Common</td>
                            <td id="otherCommonPrice"></td>
                            <td id="otherCommonShares"></td>
                            <td id="otherCommonOwnership"></td>

                        </tr>';

        $page .= '      <tr><td colspan="4"><hr></td></tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Preferred</td>
                            <td id="preferredPrice" style="font-weight: bold"></td>
                            <td id="preferredShares" style="font-weight: bold"></td>
                            <td id="preferredOwnership" style="font-weight: bold"></td>

                        </tr>';

        $page .= '      <tr><td style="text-align: right"><span title="Show Price Calc" ><a onClick="displayEquation(this,\'vcEquationDiv\')">VC</a></span></td>
                            <td id="vcPrice"></td>
                            <td id="vcShares"></td>
                            <td id="vcOwnership"></td>
                        </tr>';

        $page .= '      <tr><td style="border-right: 0px;" colspan="5">
                            <table class="angelCapTable" id="convertOwnershipTable"><thead>
                            <tr><th style="text-align:right; color: #1574B1; width:200px"></td>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px"></th>
                                    <th style="width:125px; text-align:right; border: 0px;"></th>
                             </tr></thead><tbody></tbody></table>
                         </td></tr>';



        $page .= '      <tr><td colspan="4"><hr></td></tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Options Pool</td>
                            <td id="optionsPrice">N/A</td>
                            <td id="optionsShares"></td>
                            <td id="optionsOwnership"></td>
                        </tr>';

        $page .= '      <tr><td colspan="4"><hr></td></tr>';
        $page .= '      <tr><td colspan="4"><hr></td></tr>';

        $page .= '      <tr><td style="color: #1574B1; text-align: left">Your Equity</td>
                            <td id="yourPrice" style="font-weight: bold"></td>
                            <td id="yourShares" style="font-weight: bold"></td>
                            <td id="yourOwnership" style="font-weight: bold"></td>

                        </tr>';

        $page .= '      <tr><td  style="border-right: 0px;" colspan="5">
                            <table class="angelCapTable" id="ownershipTable"><thead>
                            <tr><th style="color: #1574B1; width:200px; text-align: left"></th>
                                    <th style="width:125px" style="font-weight: bold"></th>
                                    <th style="width:125px" style="font-weight: bold"></th>
                                    <th style="width:125px" style="font-weight: bold"></th>
                                    <th style="width:125px; text-align:right; border: 0px;"></th>
                             </tr></thead><tbody></tbody></table>
                         </td></tr>';

        $page .= '     </table>';   // captable
        $page .= '</div>';          // financing Details

        $page .= '<div id="equationDiv" style="float: left; margin: 30px 0px 0px 20px; display: block;">';

        $page .=  '
                       <table id="equationTable">
                         <tr><td>
                           <div class="angelEquationDiv" id="vcEquationDiv" style="display:none">
                            <span style="text-align: left; color: #1574B1">VC Calculation</span>
                            <table class="angelEquationTable" style="width: 300px;" >

                                <tr><td style="width:50px"></td>  <td style="width:200px;" id="vcEquationNum">Numerator</td></tr>

                                <tr><td id="vcPriceEquals" style="text-align: right; padding-right: 10px; width: 50px;">price =</td>
                                    <td><hr></td></tr>
                                <tr><td></td><td style="width:200px" id="vcEquationDenom">Denominator</td></tr>

                             </table></div>
                         </td></tr>
                         <tr><td>
                           <div class="angelEquationDiv" id="angelEquationDiv" style="display:none">
                            <table class="angelEquationTable" width="300px">
                                <tr><td colspan="2" style="text-align: left; color: #1574B1">Angel Calculation</td></tr>

                                <tr><td></td><td id="your0EquationNum">Numerator</td></tr>
                                <tr><td id="angelPriceEquals" style="text-align: right; padding-right: 10px">price =</td><td><hr></td></tr>
                                <tr><td></td><td  width="400px" id="your0EquationDenom">Denominator</td></tr>
                            </table></div>
                         </td></tr>
                       </table>';

        $page .= '</div>';          // equation div.
        $page .= '</div>';          // output div.
        $page .= '<div style="clear:both"></div>';

        return($page);
    }   // convertpageoutput()

    /****************************************************************
 * @param $investorID
 * @return string
 */

    function modelComparePageBody($investorID) {

        $path = $GLOBALS["path"];
        $page = "<script>$(document).ready( function() { initFeedback('right-bottom', $investorID) } );</script>";

        $page .=  "<form id='modelChooserForm' action='$path' method='post' enctype='multipart/form-data'>";
        $mod = new angelModel();
        $conn = $mod->connect();

        $modelRows = $mod->getRows("modeltable", "investor_id=$investorID"); // get this guy's models, if any

        $list = $this->createModelSelectList($modelRows);

    /*
     * Output - cap table models to compare
     */
        $page .= '
                 <div id="outputDiv" class="modelCompareTableDiv">
                    <table class="modelCompareTable">
                         <th style=" width:140px"></th>
                         <th style=" width:140px">Model 1</th>
                         <th style=" width:140px">Model 2</th>
                         <th style=" width:140px">Mod 1 - Mod 2</th>
                         <th style=" text-align:center;width:300px">Model 1 (Shares)</th>
                         <th style=" text-align:center;width:300px">Model 2 (Shares)</th>



                        <tr><td>Valuation:</td>              <td id="postVal1"></td>           <td id="postVal2"></td>        <td style="font-weight: bold" id="postValD"></td>
                        <td rowspan="10"><div id="model1-chart"></div> </td><td rowspan="10"><div id="model2-chart"></div></td>
                        </tr>
                        <tr><td>Total FD Shares:</td>        <td id="postFDShares1"></td>      <td id="postFDShares2"></td>   <td style="font-weight: bold" id="postFDSharesD"></td></tr>
                        <tr><td>Pre-money FD Shrs:</td>        <td id="preFDShares1"></td>      <td id="preFDShares2"></td>   <td style="font-weight: bold" id="preFDSharesD"></td></tr>
                        <tr><td>Share Price:</td>            <td id="postPrice1"></td>         <td id="postPrice2"></td>      <td style="font-weight: bold" id="postPriceD"></td></tr>
                        <tr><td colspan=4>&nbsp</td></tr>
                        <tr><td>VC Ownership:</td>           <td id="postVCOwnership1"></td>    <td id="postVCOwnership2"></td>      <td style="font-weight: bold" id="postVCOwnershipD"></td></tr>
                        <tr><td>Convert Ownership:</td>      <td id="postConvOwnership1"></td>  <td id="postConvOwnership2"></td>    <td style="font-weight: bold" id="postConvOwnershipD"></td></tr>
                        <tr><td>Founder Ownership:</td>       <td id="postFounderOwnership1"></td><td id="postFounderOwnership2"></td>  <td style="font-weight: bold" id="postFounderOwnershipD"></td></tr>
                        <tr><td>Options:</td>                <td id="postOptionsPercent1"></td> <td id="postOptionsPercent2"></td>   <td style="font-weight: bold" id="postOptionsPercentD"></td></tr>
                        <tr><td colspan=4>&nbsp</td></tr>
                        <tr><td>Angel:</td>                  <td id="postYourOwnership1"></td> <td id="postYourOwnership2"></td>   <td style="font-weight: bold" id="postYourOwnershipD"></td></tr>
                    </table>
                 </div>';


        // $page .= "<div style='clear:both'></div>";
        /*
         * Input: models to compare
         */
        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'>";  // container and clear

        $page .= "
                            <a href='javascript:onclick=modelCompare()' class='modelButton'>Compare</a>

                            </div>
                            ";

        /*
         * Model 1 on the left
         */
        $page .= "<div class='modelCompareDiv'>";
        $page .= "<span style='font-size:14px; font-weight: bold '> Model 1: </span>";
        $page .= "<select name='model1' onchange=loadCompareModel(this,'1')><option value=0>-- choose --</option>";
        $page .= $list;
        $page .= "</select>";
        $page .= $this->modelInputForm("1");
        $page .= "</div>";

        /*
         * Model 2 on the right
         */
        $page .= "<div class='modelCompareDiv'>";
        $page .= "<span style='font-size:14px; font-weight: bold '> Model 2: </span>";
        $page .= "<select name='model2' onchange=loadCompareModel(this,'2')><option value=0>-- choose --</option>";
        $page .= $list;
        $page .= "</select>";
        $page .= $this->modelInputForm("2");
        $page .= "</div>";

        $page .= "<div style='clear:both;padding: 10px 30px 10px 30px; border: 0px solid red;'>";  // container and clear

        $page .= "
                            <a href='javascript:onclick=modelCompare()' class='modelButton'>Compare</a>

                            </div>
                            ";

        return($page);

    }       /* compare */

    function modelInputForm($model) {

    $path = $GLOBALS["path"];
    $action = "model";
    $submitActionKey = "saveModel";
    $convertTable = "convertTable" . $model;
    $equityTable = "equityTable" . $model;
    $founderTable = "founderTable" . $model;
    $otherCommonTable = "otherCommonTable" . $model;
    $optionsTable = "optionsTable" . $model;
    $convertCount = 'convertCount' . $model;
    $founderCount = 'founderCount' . $model;
    $otherCommonCount = 'otherCommonCount' . $model;
    $optionsSpecified = 'optionsSpecified' . $model;
    $modelName = 'modelName' . $model;
    $modelID = 'modelID' . $model;
    $yourAmount = 'yourAmount' . $model;
    $investAmount = 'investAmount' . $model;
    $opoolPre = 'opoolPre' . $model;
    $opoolPost = 'opoolPost' . $model;
    $valuation = 'valuation' . $model;
    $valispre = 'valispre' . $model;
    $valispreName = 'pre' . $model;
    $sharesOut = 'sharesOut' . $model;
    $optionPoolSizeForm = 'optionPoolSizeForm' . $model;
    $grantedOptionsForm = 'grantedOptionsForm' . $model;
    $promisedOptionsForm = 'promisedOptionsForm' . $model;
    $unissuedOptionsForm = 'unissuedOptionsForm' . $model;




        /*
         * the input portion of the page
         */

    $page = $this->convertDialogs();

    $page .= "  <div style='clear:both'></div>
                    <div style=' border: 0px solid blue; background-color: #E0E0E0; margin:20px 0px 20px 20px; padding: 10px 30px 10px 30px; width:90%;'>

                        <form id='convertForm' action='$path/$action' method='post' enctype='multipart/form-data'>

                        <input type='hidden' name='investID' value='0'>
                        <input type='hidden' name=$submitActionKey>";

    /*
     * common stock holders, founders, YC, others
     */
    if ($model == "")
        $page .= "<span class='modelLabel'>Common Stock</span>
                      <span  style='margin: 0px 0px 0px 10px'><a href='javascript:onclick=addCommon()'>Add Common Holder</a> (add your option pool here)</span>";
    else
        $page .= "<span class='modelLabel'>Common Stock</span>";

    $page .= "<p></p>
                            <script type='text/javascript'>$(function() {
                                        $('#commonHolderShares').number(true);
                                        $('#YCCommonShares').number(true);
                                });</script>
                            <table id='$founderTable' class='convertInputTable'   style='display: none'>
                                    <thead><tr><th style='text-align: left; color:darkgrey'>Founders</th></tr>
                                    <tr><th style='width:275px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:170px'>Shares of Founder Stock</th>
                                     </tr></thead>

                                    <tbody></tbody></table><br />";
    $page .= "<p></p>

                            <table id='$otherCommonTable' class='convertInputTable'   style='display: none'>
                                    <thead><tr><th  style='text-align: left; color:darkgrey'>Other Common Holders</th></tr>
                                    <tr><th style='width:275px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:170px'>Shares of Common Stock</th>
                                    </tr></thead>

                                   <tbody></tbody></table><br />

                            <table id='$optionsTable' class='convertInputTable'   style='display: none'>
                                    <thead><tr><th colspan='2' style='text-align: left; color:darkgrey'>Options
                                    <a title='What Are These?' onClick=window.open('/static/optionshelp.html','CO','width=700,height=350,0,status=0,location=no,toolbar=no,menubar=no'); href='javascript:void(0);'><big><strong>?</strong></big></a>
                                    <span  style='margin: 0px 0px 0px 10px'>(Pool Size is the sum of Granted, Promised, and Unissued Options)</span>

                                    </th></tr>
                                    <tr><th style='width:275px; border-spacing: 0px; padding: 0px; color:black'></th>
                                    <th style='width:170px'>Shares</th>
                                    </tr></thead>

                                   <tbody>
                                   <tr>
                                        <td>Pool Size</td>
                                        <td><input type='text' onchange='modelOptionPoolForm()' id='$optionPoolSizeForm' name='optionPoolSize' value='0'></td>
                                   </tr><tr>
                                        <td>Granted Options</td>
                                        <td><input type='text' onchange='modelOptionPoolForm()' id='$grantedOptionsForm' name='grantedOptions' value='0'></td>
                                   </tr><tr>
                                        <td>Promised Options</td>
                                        <td><input type='text' onchange='modelOptionPoolForm()' id='$promisedOptionsForm' name='promisedOptions' value='0'></td>
                                   </tr><tr>
                                        <td>Unissued Options</td>
                                        <td><input type='text' onchange='modelOptionPoolForm()' id='$unissuedOptionsForm' name='unissuedOptions' value='0'></td>
                                   </tr>

                                   </tbody></table><br />
                        ";

    /*
     * convertibles
     */
    $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';

    if ($model == "") {
        $page .= " <span class='modelLabel'>Convertibles</span>
                       <span style='margin: 0px 0px 0px 10px'><a href='javascript:onclick=addConvert()'>Add Convert</a> (Note: use a 0 cap for uncapped)</span>";
    } else
        $page .= "<span class='modelLabel'>Convertibles</span>";
    $page .= "

                            <p></p>
                            <table id='$convertTable' class='convertInputTable'   style='display: none; width:95%'>

                                <tr><th style='width:160px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:150px'>Your Investment, w/Interest</th>
                                    <th style='width:100px'>Total Invested in this Convertible w/Interest</th>
                                    <th style='width:100px'>Discount (%)</th>
                                    <th style='width:100px'>Cap</th>
                                    <th style='width:200px'>Special</th>

                                </tr><tbody></tbody></table><br />";
    /*
      * equity
      */
    $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
    if ($model == "")
        $page .= " <span class='modelLabel'>Equity</span>
                       <span style='margin: 0px 0px 0px 10px'><a href='javascript:onclick=addEquityRound()'>Add Round</a></span>";
    else
        $page .= "<span class='modelLabel'>Equity</span>";

    $yAmountPound = '#' . $yourAmount;
    $iAmountPound = '#' . $investAmount;
    $valPound = '#' . $valuation;
    $sharesPound = '#' . $sharesOut;
    $oPrePound = '#' . $opoolPre;
    $page .= "

                            <p></p>
                            <table class='equityInputTable' id='$equityTable' style='border-top: 0px solid black; display:none;width:95%'>
                            <tr>
                                <th style='width:160px' >Round</th>
                                <th style='width:150px'>Your Investment (If any)</th>
                                <th style='width:100px'>Total Investment</th>
                                <th style='width:130px'>Post-Money Option Pool(%)</th>

                                <th style='width:100px'>Valuation</th>


                            </tr>
                            <tr><td style='width:100px; '>Series A</td>
                                <script type='text/javascript'>$(function() {
                                        $('$yAmountPound').number(true);
                                        $('$iAmountPound').number(true);
                                        $('$valPound').number(true);
                                        $('$sharesPound').number(true);
                                        $('$oPrePound').number(true);
                                });</script>
                                <td><input size='10%' type=text id='$yourAmount'     name='yourAmount' value='' /></td>
                                <td><input size='10%' type=text id='$investAmount'   name='investAmount' value=''/></td>
                                <td style='width:40px; border-spacing: 0px; padding: 0px '>
                                    <input size=4 type=text     id='$opoolPost'      name='opoolPost' value='' /></td>
                               <td><input size='10%' type=text id='$valuation'       name='valuation' value='' /><br />
                                      <input type='radio'       id='$valispre'       name='$valispreName' value='pre' checked> Pre
                                       <input type='radio'                          name='$valispreName' value='post'> Post</td>

                            </tr>
                            </table><br />
                            ";

        /*
         * <th style='width:140px'>Fully Diluted Pre-Money Shares
                                <a title='What does Fully Diluted Mean?' onClick=window.open('/static/fullydilutedhelp.html','CO','width=700,height=250,0,status=0,'); href='javascript:void(0);'>???</a>
                                </th>
                                <th style='width:140px'>Unissued Pre-Money Options
                                <a title='What Are These?' onClick=window.open('/static/unissuedhelp.html','CO','width=700,height=260,0,status=0,'); href='javascript:void(0);'>???</a>
                                </th>
                                   <td><input size='10%' type=text id='$sharesOut'      name='sharesOut' value='' /></td>
                                <td><input size='10%' type=text id='$opoolPre'       name='opoolPre' value=''/></td>
         */

    $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
    $page .= "<input type='hidden' id='$convertCount' name='convertCount' value='0'>";
    $page .= "<input type='hidden' id='$founderCount' name='founderCount' value='0'>";
    $page .= "<input type='hidden' id='$otherCommonCount' name='otherCommonCount' value='0'>";
    $page .= "<input type='hidden' id='$optionsSpecified' name='optionsSpecified' value='0'>";
    $page .= "<input type='hidden' name='yourPriceForm' value='0' id='yourPriceForm'>
                                <input type='hidden' name='yourSharesForm' value='0' id='yourSharesForm'>
                                <input type='hidden' name='yourOwnershipForm' value='0' id='yourOwnershipForm'>
                                <input type='hidden' name='postFDSharesForm' value='0' id='postFDSharesForm'>
                                <input type='hidden' name='postPriceForm' value='0' id='postPriceForm'>
                                <input type='hidden' name='modelName' value='' id='$modelName'>
                                <input type='hidden' name='modelID' value='0' id='$modelID'>
                                ";

    if ($model == "") {
        $page .= "



                                <a href='javascript:onclick=saveModel(\"\")' class='modelButton' id='saveButton'>Save</a>

                                <div style='float:right'>Precision for calculations:
                                <select id='precision' name='precision'>
                                    <option value='-1' selected>Maximum</option>
                                    <option value='4'>4 decimal places</option>
                                    <option value='3'>3 decimal places</option>
                                    <option value='2'>2 decimal places</option>
                                </select></div>



                            </form>
                        </div>";



        if (angelIsLoggedIn()) {
            $page .= '<p></p><H3>&nbsp &nbsp Instructions (<a href="javascript:onclick=toggleInstructions()"><span id="showHideInstructions">show</span></a>)</H3>';
            $page .= '<div id="modelInstructions" style="display:none" class="modelInstructions">';
        } else {
            $page .= '<p></p><H3>Instructions (<a href="javascript:onclick=toggleInstructions()"><span id="showHideInstructions">hide</span></a>)</H3>';
            $page .= '<div id="modelInstructions" class="modelInstructions">';
        }
        $page .= file_get_contents("includes/helpInstructions.inc");        // add instructions at the end of the page
    }
    $page .= "</div>";


        return($page);

    }       /* modelinputform */

    function modelInputFormTest($model) {

        $path = $GLOBALS["path"];
        $action = "model";
        $submitActionKey = "saveModel";
        $convertTable = "convertTable" . $model;
        $equityTable = "equityTable" . $model;
        $founderTable = "founderTable" . $model;
        $otherCommonTable = "otherCommonTable" . $model;
        $convertCount = 'convertCount' . $model;
        $founderCount = 'founderCount' . $model;
        $otherCommonCount = 'otherCommonCount' . $model;
        $modelName = 'modelName' . $model;
        $modelID = 'modelID' . $model;
        $yourAmount = 'yourAmount' . $model;
        $investAmount = 'investAmount' . $model;
        $opoolPre = 'opoolPre' . $model;
        $opoolPost = 'opoolPost' . $model;
        $valuation = 'valuation' . $model;
        $valispre = 'valispre' . $model;
        $valispreName = 'pre' . $model;
        $sharesOut = 'sharesOut' . $model;


        /*
         * the input portion of the page
         */

        $page = $this->convertDialogs();

        $page .= "  <div style='clear:both'></div>
                    <div id='inputDiv' style=' border: 0px solid blue; position: fixed; background-color: #E0E0E0; margin:270px 0px 20px 20px;
                      padding: 10px 30px 10px 30px; width:90%; height:600px; overflow:scroll'>

                        <form id='convertForm' action='$path/$action' method='post' enctype='multipart/form-data'>

                        <input type='hidden' name='investID' value='0'>
                        <input type='hidden' name=$submitActionKey>";

        /*
         * common stock holders, founders, YC, others
         */
        if ($model == "") {
            $page .= "<div style=' margin:0px 0px 0px 0px; float: left; border: 0px solid black'>";
            $page .= "<span class='modelLabel'>Common Stock</span>
                      <span  style='margin: 0px 0px 0px 10px'><a href='javascript:onclick=addCommon()'>Add</a></span></div>";
        } else
            $page .= "<span class='modelLabel'>Common Stock</span>";

        $page .= "<div  style='float: left; border: 0px solid red'><p></p>
                            <script type='text/javascript'>$(function() {
                                        $('#commonHolderShares').number(true);
                                        $('#YCCommonShares').number(true);
                                });</script>
                            <table id='$founderTable' class='convertInputTable'   style='display: none'>
                                    <thead><tr><th style='text-align: left; color:darkgrey'>Founders</th></tr>
                                    <tr><th style='width:275px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:170px'>Shares of Founder Stock</th>
                                     </tr></thead>

                                    <tbody></tbody></table><br />";
        $page .= "<p></p>

                            <table id='$otherCommonTable' class='convertInputTable'   style='display: none'>
                                    <thead><tr><th  style='text-align: left; color:darkgrey'>Other Common Holders</th></tr>
                                    <tr><th style='width:275px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:170px'>Shares of Common Stock</th>
                                    </tr></thead>

                                   <tbody></tbody></table></div><br />";

        /*
         * convertibles
         */
        $page .= "  <div style='clear:both'></div>";
        $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';

        if ($model == "") {
            $page .= " <span class='modelLabel'>Convertibles</span>
                       <span style='margin: 0px 0px 0px 10px'><a href='javascript:onclick=addConvert()'>Add</a></span>";
        } else
            $page .= "<span class='modelLabel'>Convertibles</span>";
        $page .= "

                            <p></p>
                            <table id='$convertTable' class='convertInputTable'   style='display: none; width:95%'>

                                <tr><th style='width:160px; border-spacing: 0px; padding: 0px; color:black'>Name</th>
                                    <th style='width:150px'>Your Investment, w/Interest</th>
                                    <th style='width:100px'>Total Invested in this Convertible w/Interest</th>
                                    <th style='width:100px'>Discount (%)</th>
                                    <th style='width:100px'>Cap</th>
                                    <th style='width:200px'>Special</th>

                                </tr><tbody></tbody></table><br />";
        /*
          * equity
          */
        $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
        if ($model == "")
            $page .= " <span class='modelLabel'>Equity</span>
                       <span style='margin: 0px 0px 0px 10px'><a href='javascript:onclick=addEquityRound()'>Add</a></span>";
        else
            $page .= "<span class='modelLabel'>Equity</span>";

        $yAmountPound = '#' . $yourAmount;
        $iAmountPound = '#' . $investAmount;
        $valPound = '#' . $valuation;
        $sharesPound = '#' . $sharesOut;
        $oPrePound = '#' . $opoolPre;
        $page .= "

                            <p></p>
                            <table class='equityInputTable' id='$equityTable' style='border-top: 0px solid black; display:none;width:95%'>
                            <tr>
                                <th style='width:160px' >Round</th>
                                <th style='width:150px'>Your Investment (If any)</th>
                                <th style='width:100px'>Total Investment</th>
                                <th style='width:130px'>Post-Money Option Pool(%)</th>

                                <th style='width:100px'>Valuation</th>
                                <th style='width:140px'>Fully Diluted Pre-Money Shares</th>
                                <th style='width:140px'>Unallocated Pre-Money Options</th>

                            </tr>
                            <tr><td style='width:100px; '>Series A</td>
                                <script type='text/javascript'>$(function() {
                                        $('$yAmountPound').number(true);
                                        $('$iAmountPound').number(true);
                                        $('$valPound').number(true);
                                        $('$sharesPound').number(true);
                                        $('$oPrePound').number(true);
                                });</script>
                                <td><input size='10%' type=text id='$yourAmount'     name='yourAmount' value='' /></td>
                                <td><input size='10%' type=text id='$investAmount'   name='investAmount' value=''/></td>
                                <td style='width:40px; border-spacing: 0px; padding: 0px '>
                                    <input size=4 type=text     id='$opoolPost'      name='opoolPost' value='' /></td>
                               <td><input size='10%' type=text id='$valuation'       name='valuation' value='' /><br />
                                      <input type='radio'       id='$valispre'       name='$valispreName' value='pre' checked> Pre
                                       <input type='radio'                          name='$valispreName' value='post'> Post</td>
                                <td><input size='10%' type=text id='$sharesOut'      name='sharesOut' value='' /></td>
                                <td><input size='10%' type=text id='$opoolPre'       name='opoolPre' value=''/></td>
                            </tr>
                            </table><br />
                            ";

        // $page .= '<div style="width:110%; margin:0px 0px 20px -30px; height: 10px; background-color:#F6F6EA"></div>';
        $page .= "<input type='hidden' id='$convertCount' name='convertCount' value='0'>";
        $page .= "<input type='hidden' id='$founderCount' name='founderCount' value='0'>";
        $page .= "<input type='hidden' id='$otherCommonCount' name='otherCommonCount' value='0'>";
        $page .= "<input type='hidden' name='yourPriceForm' value='0' id='yourPriceForm'>
                                <input type='hidden' name='yourSharesForm' value='0' id='yourSharesForm'>
                                <input type='hidden' name='yourOwnershipForm' value='0' id='yourOwnershipForm'>
                                <input type='hidden' name='postFDSharesForm' value='0' id='postFDSharesForm'>
                                <input type='hidden' name='postPriceForm' value='0' id='postPriceForm'>
                                <input type='hidden' name='modelName' value='' id='$modelName'>
                                <input type='hidden' name='modelID' value='0' id='$modelID'>
                                ";

        if ($model == "") {
            $page .= " </form></div>";

        }
        $page .= "</div>";


        return($page);

    }       /* modelinputform */


    function compareInputForm2($model) {

        $formName = "convertForm" . $model;
        $convertTable = "convertTable" . $model;
        $equityTable = "equityTable" . $model;
        $cCount = 'convertCount' . $model;
        $mName = 'modelName' . $model;
        $mID = 'modelID' . $model;
        $yAmount = 'yourAmount' . $model;
        $iAmount = 'investAmount' . $model;
        $oPre = 'opoolPre' . $model;
        $oPost = 'opoolPost' . $model;
        $val = 'valuation' . $model;
        $valIsPre = 'valispre' . $model;
        $shares = 'sharesOut' . $model;

        // <span style='color:blue; border:0px solid blue'>Convertibles -- <span><a href='javascript:onclick=addConvert()'>Add New</a></span>
        $form = "<form>

                            <p></p>
                            <table id='$convertTable' class='convertCompareInputTable'   style='display: none'>

                                <tr><th style='width:150px; border-spacing: 0px; padding: 0px; color:black'></th>
                                    <th style='width:150px'>Your Investment, Incl Interest</th>
                                    <th style='width:100px'>Total Invested in this Convertible, Incl Interest</th>
                                    <th style='width:100px'>Discount (%)</th>
                                    <th style='width:100px'>Cap</th>



                                </tr></table><br />";

        $yAmountPound = '#' . $yAmount;
        $iAmountPound = '#' . $iAmount;
        $valPound = '#' . $val;
        $sharesPound = '#' . $shares;
        $oPrePound = '#' . $oPre;

        $form .= "
                    <table class='equityCompareTable' id='$equityTable' style='border-top: 0px solid black;display: none'>
                    <tr>
                        <th style='width:100px; color:blue; width:80px; border-spacing: 0px; padding: 0px ' ></th>
                        <th style='width:150px'>Your Investment (If any)</th>
                        <th style='width:100px'>Total Investment</th>
                        <th style='width:130px'>Post-Money Option Pool</th>

                        <th style='width:120px'>Valuation</th>
                        <th style='width:140px'>Fully Diluted Pre-Money Shares</th>
                        <th style='width:120px'>Unallocated Pre-Money Options</th>

                    </tr>
                    <script type='text/javascript'>$(function() {
                                        $('$yAmountPound').number(true);
                                        $('$iAmountPound').number(true);
                                        $('$valPound').number(true);
                                        $('$sharesPound').number(true);
                                        $('$oPrePound').number(true);
                                });</script>
                    <tr><td style='width:100px; background-color:white; color:black; font-size:12px;border-spacing: 0px; padding: 0px '>Qualified Financing</td>
                        <td><input size='7%' type=text id='$yAmount'  name='yourAmount' value=''></td>
                        <td><input size='9%' type=text id='$iAmount'  name='investAmount' value=''></td>

                        <td style='width:20px; border-spacing: 0px; padding: 0px '>
                            <input size=4 type=text id='$oPost'       name='opoolPost' value=''></td>
                        <td><input size='10%' type=text id='$val'      name='$val' value=''><br />
                            <input type='radio' id='$valIsPre' name='pre' value='pre' checked> Pre <input type='radio' name='pre' value='post'>Post</td>

                        <td><input size='10%' type=text id='$shares'  name='sharesOut' value=''></td>
                        <td><input size='9%' type=text id='$oPre'    name='opoolPre' value='' ></td>
                    </tr>
                    </table><p></p>
                    ";


        $form .= "<input type='hidden' id='$cCount' name='convertCount' value='0'>";

        $form .= "
                            <input type='hidden' id='$mName' value='$modelName' name='modelName'>
                            <input type='hidden' id='$mID' value='$modelID' name='modelID'>


                        </form>";

        return($form);

    }

    function createModelSelectList($modelRows) {
        $list = "";

        if ($modelRows) {
            while($m = mysqli_fetch_object($modelRows,"angelModel")) {    // retrieve all the saved models for this user
                $list .= "<option value=$m->id>$m->name</option>";                 // and allow them to be selected
            }
        }

        if ($list == "")
            $list = "<option value=-1>Sample Model</option>";

        return($list);

    }

    function getInvestments($conn, $co) {

        $query = "SELECT * from Investmenttable WHERE company_id=$co->id";
        angeldie("getInvestments: " . $query);    // debug

    }



    /*
     * return new investment form for input person ($id)
     */
    function newInvestmentForm($id, $conn) {

        $cos = array();
        $vcs = array();
        $angels = array();
        ik12_getInvestorMap($cos, $conn, 'startup');
        ik12_getInvestorMap($vcs, $conn, 'vc');
        ik12_getInvestorMap($angels, $conn, 'angel');
        $hideInvestors = '';
        $hideAcquirer = 'hidden';

        $path = $GLOBALS["path"];

        $rounds = getAngelRounds($conn);

        $invForm = "";

        $invForm .= '<div class="angelinput">';
        $invForm .= "<form id='addInvestmentForm' action='/investments.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";
        $invForm .= "<input type=hidden value=$id name='id'>";
        /*
         * make the format of numbers input into text fields look nice
         */
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#invest_amount').number(true);
                $('#prefShares').number(true);
                $('#commonShares').number(true);

            });</script>";


        $invForm .= "<input type=hidden value=$id name='add'>";



        $invForm .= "<table style='border-bottom: 0px solid blue' width='1000px'><thead>
        <tr>
            <th id='dateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='companyHeader' valign=top width='10%'>Company<span style='color:red'>*</span></th>
            <th id='roundHeader' valign=top width='10%'>Round<span style='color:red'>*</span></th>
            <th id='typeHeader' valign=top width='10%'>Type<span style='color:red'>*</span></th>
            <th id='amountHeader' valign=top width='10%'>Amount<span style='color:red'>*</span></th>
            <th id='valHeader' valign=top width='10%'>Valuation Cap</th>
         </tr></thead>";


        $inv = new angelInvest();

        $selectedRound = 'seed';            // default for a new investment is Seed
        $selectedType = 'safe';         // default for an investment type is a SAFE
        $notes = '';


        $invForm .= "<tbody>";
        $invForm .= "<tr><td valign=top><input size='11%' type=text id='datepickerInv' name='date' value=''></td>";


        /*
         * Company
         */

        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_cos = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#e5").select2({
                placeholder: "Select a Company",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


                    $needComma = false;

                    $invForm .= "data: [";
                    foreach (array_keys($cos) as $id) {
                       $co = mysqli_escape_string($conn, $cos[$id]);
                       if ($needComma)
                            $invForm .= ",";

                       $invForm .= "{id:$id, text:'$co'}";

                       $needComma = true;
                    }
                    $invForm .= "]";


            $invForm .= '});
            $("#e5").select2("data", preload_data_cos);';

        $invForm .= '});
    </script>';
    $invForm .= '<input type="hidden" id="e5" name="company" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">For a new company just enter its name</span>
           </td>';

        /*
         * Round
         */

        $invForm .= "<script>preload_data = 0;";
        $invForm .= '
        $(document).ready(function () {
            $("#round").select2({
                placeholder: "Round?",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach (array_keys($rounds) as $id) {
            $round = mysqli_escape_string($conn, $rounds[$id]);
            $round = ucwords($round);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$round'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#round").select2("data", preload_data);';

        /*
         * Let's save an ajax call by allowing the Rounds to be easily accessed by id from javascript
         */
        foreach (array_keys($rounds) as $id) {
            $round = mysqli_escape_string($conn, $rounds[$id]);
            $invForm .= "RoundMap['$round'] = $id;";
        }
        $invForm .= '});</script>';




        $invForm .= '<td><input type="hidden" id="round" name="round" onchange="newInvestRoundToType();" style="width:200px"/><br/>
                 <span style="font-size:10px;text-transform:none">For a custom round just enter it</span>
           </td>';


        /*
         * Type
         */

        $invForm .= '<td valign=top><select id="investType" name="type">';
        foreach ($GLOBALS["investmentTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';

        $invForm .= "<script>$('#investType').bind('change', function(event) {

            var i= $('#investType').val();

            if (i=='equity' || i =='other') {
                $('#valHeader').html('Pre-money Valuation');
                $('#prevaluation').number(true);
                $('#amountHeader').html('Amount');
            } else if (i=='safe' || i=='note') {
                $('#valHeader').html('Valuation Cap');
                $('#amountHeader').html('Amount');
                $('#prevaluation').number(true);
            } else if (i=='warrant' || i=='option') {
                $('#valHeader').html('Strike Price');
                $('#amountHeader').html('Shares');
                $('#prevaluation').number(true,4);

            }

          });</script>";

        $invForm .= "

            <td valign=top><input size='10%' type=text id='invest_amount'  name='invest_amount'></td>
            <td valign=top><input size='10%' type=text id='prevaluation'  name='prevaluation'></td></tr>";
        $invForm .= "<tr><td style='font-family: Verdana,Arial;font-size:12px;font-weight: bold'>NOTES: </td><td colspan=5 valign=top><textarea rows=5 cols=120 id='notes' name='notes'>$notes</textarea></td></tr></table>";


        $invForm .= '<a href="javascript:onclick=$(\'#invEquityDetails\').css(\'display\',\'block\');$(\'#enterEquityText\').css(\'display\',\'none\');">
                <span id="enterEquityText">Click to Enter Equity Details</span></a>';
        $invForm .="<div id='invEquityDetails' style='display: none'>

        <table style='border-bottom: 2px solid blue; padding: 15px;' width='1000px'><thead>
       <tr>
           <th id='' valign=top width='10%'></th>
           <th id='' valign=top width='10%'>Preferred Shares</th>
           <th id='' valign=top width='10%'>Preferred Series</th>
           <th id='' valign=top width='10%'>Preferred Cert No.</th>
           <th style='background-color: #c0c0e0'>&nbsp</th>
           <th id='' valign=top width='10%'>Common Shares</th>
           <th id='' valign=top width='10%'>Common Cert No.</th>
           <td style='background-color: #c0c0e0'>&nbsp</td>
           <th id='' valign=top width='10%'>Cert Loc</th>
        </tr></thead>";
        $invForm .= "
            <tr><td style='font-family: Verdana,Arial;font-size:12px;font-weight: bold'>EQUITY: </td>
            <td valign=top><input size='10%' type=text id='prefShares'  name='prefShares' value=''></td>";

        $invForm .= "<script>preload_data = 0;";
        $invForm .= '
        $(document).ready(function () {
            $("#prefSharesSeries").select2({
                placeholder: "Series?",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach (array_keys($rounds) as $id) {
            $round = mysqli_escape_string($conn, $rounds[$id]);
            if ($round == "seed")
                continue;
            $round = ucwords($round);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$round'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});

            $("#prefSharesSeries").select2("data", preload_data);

            ';

        $invForm .= '});</script>';


        $invForm .= '<td><input type="hidden" id="prefSharesSeries" name="prefSharesSeries" style="width:200px"/></td>';

        $invForm .= "
            <td valign=top><input size='10%' type=text id='prefCertNum'  name='prefCertNum' value=''></td>

            <td style='background-color: #c0c0e0'>&nbsp</td>

            <td valign=top><input size='10%' type=text id='commonShares'  name='commonShares' value=''></td>
            <td valign=top><input size='10%' type=text id='commCertNum'  name='commCertNum' value=''></td>
            <td style='background-color: #c0c0e0'>&nbsp</td>
            <td valign=top><input size='10%' type=text id='certLoc'  name='certLoc' value=''></td>
            <td valign=top><input type=checkbox id='converted'  name='converted' value=''>This was a convert</td>
            </tr></table></div>";




        $invForm .= "<input type='hidden' id='update' name='update' value='0'>";
        $invForm .= "<input type='hidden' id='action' name='action' value='add'>";
        $invForm .= '<p></p><button type="button" id="newInvestmentButton" onclick="addOrUpdateInvestment()">Add New Investment</button></form>';
        $invForm .= '</div>';   // angelinput

        return $invForm;

    }  /* newInvestmentForm */

    /*
   * return new investment form for input person ($id)
   */
    function newTrackerForm($id, $conn) {

        $cos = array();
        $people = array();

        ik12_getInvestorMap($cos, $conn, 'startup');
        ik12_createPersonMap($people, $conn);


        $hideInvestors = '';
        $hideAcquirer = 'hidden';

        $path = $GLOBALS["path"];

        $invForm = "";

        $invForm .= '<div class="angelinput">';
        $invForm .= "<form id='addTrackerForm' action='/tracker.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";
        $invForm .= "<input type=hidden value=$id name='id'>";


        $invForm .= "<input type=hidden value=$id name='add'>";



        $invForm .= "<table style='border-bottom: 0px solid blue' width='1000px'><thead>
        <tr>

            <th id='trtypeHeader' valign=top width='10%'>What to track?<span style='color:red'>*</span></th>

            <th id='trcompanyOrPersonHeader' valign=top width='10%'>Company<span style='color:red'>*</span></th>
            <th id='trfrequencyHeader' valign=top width='10%'>How often?<span style='color:red'>*</span></th>

            <th id='trstartHeader' valign=top width='10%'>Starting when?</th>


         </tr></thead>";




        $selectedRound = 'seed';            // default for a new investment is Seed
        $selectedType = 'safe';         // default for an investment type is a SAFE
        $notes = '';


        $invForm .= "<tbody>";

        /*
         * Type
         */

        /*$invForm .= '<tr><td valign=top><select id="trackerType" name="type">';
        foreach ($GLOBALS["trackerTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';*/



        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_types = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#trackerType").select2({
                placeholder: "What to Track?",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach ($GLOBALS["trackerTypes"] as $t) {
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:'$t', text:'$t'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#trackerType").select2("data", preload_data_types);';

        $invForm .= '}); </script>';
        $invForm .= '<input type="hidden" id="trackerType" name="type" style="width:300px"/>

          ';

        $invForm .= "<script>$('#trackerType').bind('change', function(event) {

            var i= $('#trackerType').val();

            if (i=='person') {
                $('#trpersonDiv').css('display','block');
                $('#trcompanyDiv').css('display','none');
                $('#trcompanyOrPersonHeader').html('Person<span style=\"color:red\">*</span></th>');
            } else if (i=='company') {
                $('#trpersonDiv').css('display','none');
                $('#trcompanyDiv').css('display','block');
                $('#trcompanyOrPersonHeader').html('Company<span style=\"color:red\">*</span></th>');
            }

          });</script></td>";

        /*
         * Company
         */

        $invForm .= "<td valign=top width='15%'><div id='trcompanyDiv'>
            <script>";
        $invForm .= "preload_data_cos = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#trcompany").select2({
                placeholder: "Choose a Company",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach (array_keys($cos) as $id) {
            $co = mysqli_escape_string($conn, $cos[$id]);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$co'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#trcompany").select2("data", preload_data_cos);';

        $invForm .= '}); </script>';
        $invForm .= '<input type="hidden" id="trcompany" name="company" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">For a new company just enter its name</span>
           </div>';

        /*
         * Person
         */
       // $invForm .= "<td valign=top width='15%'>";
        $invForm .= "<script>preload_data = 0;";
        $invForm .= '
        $(document).ready(function () {
            $("#trperson").select2({
                placeholder: "Choose a Person",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach (array_keys($people) as $id) {
            $name = mysqli_escape_string($conn, $people[$id]);
            $name = ucwords($name);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$name'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});    $("#trperson").select2("data", preload_data);';

        $invForm .= '});</script>';


        $invForm .= '<div id="trpersonDiv" style="display:none"><input type="hidden" id="trperson" name="person" ;" style="width:200px"/><br/>
                 <span style="font-size:10px;text-transform:none">For a new person just enter their name</span>
           </td></div></div>';

        /*
         * How Often
         */
        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_frequencies = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#trackerFrequency").select2({
                placeholder: "Choose how often",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach ($GLOBALS["trackerFrequencies"] as $t) {
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:'$t', text:'$t'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#trackerFrequency").select2("data", preload_data_frequencies);';

        $invForm .= '}); </script>';
        $invForm .= '<input type="hidden" id="trackerFrequency" name="frequency" style="width:300px"/>

          ';

        $invForm .= "</td>";
        /*
         * When to start
         */
        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_starts = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#trackerStart").select2({
                placeholder: "Starting When?",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach ($GLOBALS["trackerStarts"] as $t) {
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:'$t', text:'$t'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#trackerStart").select2("data", preload_data_starts);';

        $invForm .= '}); </script>';
        $invForm .= '<input type="hidden" id="trackerStart" name="start" style="width:300px"/>

          ';

        $invForm .= "</td>";
        /*
        $invForm .= "<td valign=top><input size='11%' type=text id='datepickerTra' name='date' value=''></td>";*/
        $invForm .= "</tr>";


        $invForm .= "<tr><td  colspan=6 >&nbsp</td>";

        $invForm .= "<tr>
                              <td style='font-family: Verdana,Arial;font-size:12px;font-weight: bold;'> NOTES: </td>
                               <td colspan=6 valign=top style='font-family: Verdana,Arial;'>
                               <textarea rows=5 cols=120 id='trackernotes' name='trackerNotes'>$notes</textarea></td></tr></table>";


        $invForm .= "<input type='hidden' id='trackerupdate' name='update' value='0'>";
        $invForm .= "<input type='hidden' id='trackeraction' name='action' value='addTracker'>";
        $invForm .= '<p></p><button type="button" id="newTrackerButton" onclick="addOrUpdateTracker()">Add New Tracker</button></form>';
        $invForm .= '</div>';   // angelinput

        return $invForm;

    }  /* newTrackerForm */




    function moreInvestmentInfo() {
        $invForm =  "<div class='add' id='moreInfo'>";


        $invForm .= "<table width='100%'><thead>
        <tr>
            <th valign=top width='26%'>Other Investors
            <th valign=top width='15%'>Discount (%)
            <th valign=top width='15%'>Shares
            <th valign=top width='15%'>Cert Num
            <th valign=top width='15%'>Cert loc
            <th valign=top width='15%'>Price
            <th valign=top width='15%'>Post Price
            <th valign=top width='15%'>FD Shares
         </tr></thead>";

        $invForm .= "
            <td valign=top><div id='investorsDIV' $hideInvestors><table><tr><td>VCs:</td>";

        /*
         * Other Investors: VC's and Angel's -- I may kill this
         */
        $invForm .= "
        <td>
            <script>";


        $invForm .= "preload_data = [];";
        $invForm .= '
                $(document).ready(function () {
                    $("#e3").select2({
                        placeholder: "Select an Investor",
                        createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                        multiple: true,';


        $needComma = false;

        $invForm .= "data: [";
        foreach (array_keys($vcs) as $id) {
            $vc = mysqli_escape_string($conn, $vcs[$id]);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$vc'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
                    $("#e3").select2("data", preload_data);';

        $invForm .= '});
            </script>';
        $invForm .= '<input type="hidden" id="e3" name="vcs[]" style="width:300px"/>
        </td>';

        $invForm .= '

                                 <tr><td>Angels:</td>';


        $invForm .= "
        <td>
            <script>";


        $invForm .= "preload_data_angels= [];";
        $invForm .= '
                $(document).ready(function () {
                    $("#e4").select2({
                        placeholder: "Select an Investor",
                        createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                        multiple: true,';

        $needComma = false;
        $invForm .= "data: [";
        foreach (array_keys($angels) as $id) {
            $angel = mysqli_escape_string($conn, $angels[$id]);

            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$angel'}";

            $needComma = true;
        }
        $invForm .= "]";

        $invForm .= '
                    });
                    $("#e4").select2("data", preload_data_angels);

                });
            </script>';

        $invForm .= '
                    <input type="hidden" id="e4" name="angels[]" style="width:300px"/>
                    </td>';


        $acquirer = "";     // not sure about this!

        $invForm .= '</tr></table>';
        $invForm .= '<span style="font-size:10px;text-transform:none">For new investors just enter their name(s).</span></div>';
        $invForm .= "<div id='acquirerDIV' style='text-transform:uppercase;font-weight:bold; font-family:Verdana,Arial,sans-serif' $hideAcquirer>
                      Acquirer: <input size='15%' name='acquirer' value='$acquirer'> </div></td>";

        $invForm .= "<td valign=top><input size='10%' type=text id='discount' pattern='[+]?[0-9]*[.,]?[0-9]+' name='discount' value='$inv->discount'></td>";
        $invForm .= "<td valign=top><input size='10%' type=text id='shares' pattern='[+]?[0-9]*[.,]?[0-9]+' name='shares' value=''></td>";
        $invForm .= "<td valign=top><input size='10%' type=text id='certNum' name='certNum' value=''></td>";
        $invForm .= "<td valign=top><input size='10%' type=text id='certLoc'  name='certLoc' value=''></td>";
        $invForm .= "<td valign=top><input size='10%' type=text id='price' pattern='[+]?[0-9]*[.,]?[0-9]+' name='price' value=''></td>";
        $invForm .= "<td valign=top><input size='10%' type=text id='postPrice' pattern='[+]?[0-9]*[.,]?[0-9]+' name='postPrice' value=''></td>";
        $invForm .= "<td valign=top><input size='10%' type=text id='FDShares' pattern='[+]?[0-9]*[.,]?[0-9]+' name='FDShares' value='$inv->total_fdshares'></td>";

        $invForm .= "</tbody></table></div>";

    }

    /*
     * return new payout form for input person ($id) filled in for investment being edited
     */
    function newPayoutForm($id, $conn) {


        $myCos = array();
        angelGetMyCompanies($myCos, $id);
        //angeldie(serialize($myCos));
        $invForm = "";



        $invForm .= "<form id='addPayoutForm' action='/investments.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";
        $invForm .= "<input type=hidden value=$id name='id'>";
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#payoutCash').number(true);
                $('#payoutShares').number(true);
            });</script>";


        $invForm .= "<input type=hidden value=$id name='add'>";

        $invForm .= '<div class="angelinput">';

        $invForm .= "<table width='100%'><thead>
        <tr>
            <th id='payoutDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='payoutCompanyHeader' valign=top width='10%'>Original Investment In<span style='color:red'>*</span></th>
            <th id='payoutTypeHeader' valign=top width='10%'>Type of Dist<span style='color:red'>*</span></th>
            <th id='payoutAmountHeader' valign=top width='10%'>Amount<span style='color:red'>*</span></th>
            <th id='payoutPriceHeader' valign=top width='10%' style='display:none'>Price of Record<span style='color:red'>*</span></th>
            <th id='payoutSymbolHeader' valign=top width='10%' style='display:none'>Symbol</th>
            <th id='payoutNotesHeader' valign=top width='20%'>Notes
         </tr></thead>";


        $inv = new angelInvest();

        $selectedType = 'Note';         // default for an investment type is a note
        $notes = '';

        $invForm .= "<tbody>";
        $invForm .= "<tr><td valign=top><input size='11%' type=text id='datepickerPay' name='date' value=''></td>";
        $invForm .= "<td valign=top width='15%'>
            <script>";

        /*
         * Company
         */


        $invForm .= "preload_data_cos = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#payoutCompany").select2({
                placeholder: "Select a Company",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach ($myCos AS $co) {
            //$co = mysqli_escape_string($conn, $myCos[$id]);
            $id = $co["id"];
            $name = mysqli_escape_string($conn,$co["name"]);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$name'}";

            $needComma = true;
        }
        $invForm .= "]";

        $invForm .= '});
            $("#payoutCompany").select2("data", preload_data_cos);';

        $invForm .= '});
    </script>';
        $invForm .= '<input type="hidden" onchange="payoutAddTaxInfo(0)" id="payoutCompany" name="company" style="width:300px"/><br>
                     <input type="checkbox" name="lastPayout"> I think this will be the last distribution.
           </td>';

        /*
         * Type
         */

        $invForm .= '<td valign=top><select id="payoutType" name="type">';
        foreach ($GLOBALS["payoutTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';

        $invForm .= "<script>$('#payoutType').bind('change', function(event) {

            var i= $('#payoutType').val();
            // console.debug('change!: ' + i);
            if(i =='stock') {
                // console.debug('payouts - here');
                $('#payoutAmountHeader').html('No. Shares<span style=\"color:red\">*</span>');
                $('#payoutPriceHeader').css('display','table-cell');
                $('#payoutSymbolHeader').css('display','table-cell');
                $('#payoutPriceCell').css('display','table-cell');
                $('#payoutSharesCell').css('display','table-cell');
                $('#payoutSymbolCell').css('display','table-cell');

                $('#payoutCashCell').css('display','none');
            } else {
                $('#payoutAmountHeader').html('Amount<span style=\"color:red\">*</span>');
                $('#payoutPriceHeader').css('display','none');
                $('#payoutSymbolHeader').css('display','none');
                $('#payoutPriceCell').css('display','none');
                $('#payoutSharesCell').css('display','none');
                $('#payoutSymbolCell').css('display','none');

                $('#payoutCashCell').css('display','table-cell');

            }

          });</script>";


        $invForm .= "<td  id='payoutCashCell' valign=top><input size='10%' type=text id='payoutCash'  name='payoutCash' value=''></td>";

        $invForm .= "<td id='payoutSharesCell' valign=top style='display:none'><input size='10%' type=text id='payoutShares'  name='payoutShares' value=''></td>";
        $invForm .= "<td id='payoutPriceCell' valign=top style='display:none'><input size='10%' type=text id='payoutPrice'  name='payoutPrice' value=''></td>";
        $invForm .= "<td id='payoutSymbolCell' valign=top style='display:none'><input size='10%' type=text id='payoutSymbol'  name='payoutSymbol' value=''></td>";


           // <td valign=top><input size='10%' type=checkbox id='lastPayout'  name='lastPayout' value=''></td>";
        $invForm .= "<td valign=top><textarea rows=5 cols=30 id='payoutNotes' name='payoutNotes'>$notes</textarea></td></tr>";
        $invForm .= "<tr><td colspan=7><div id='payoutTaxInfoDiv'></div></td></tr>";
        $invForm .= "</table>";


        $invForm .= "<input type='hidden' id='payoutUpdate' name='update' value='0'>";
        $invForm .= "<input type='hidden' id='payoutAction' name='action' value='addPayout'>";
        $invForm .= '<p></p><button type="button" id="newPayoutButton" onclick="addOrUpdatePayout(\'\')">Add New Distribution</button></form>';
        $invForm .= '</div>';   // angelinput


        return $invForm;

    }  /* newPayoutForm */

    /*
  * return new fund investment form for input person ($id)
  */
    function newFundInvestForm($id, $conn) {

       /*
        * Create funds map (name, id)
        */
        $funds = array();
        $f = new angelFund();
        $f->shareconn($conn);
        $f->createMap($funds,"fundtable", "id", "name");

        $path = $GLOBALS["path"];

        $rounds = getAngelRounds($conn);

        $invForm = "";

        $invForm .= '<div class="angelinput">';
        $invForm .= "<form id='addFundForm' action='/investments.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";
        /*
         * make the format of numbers input into text fields look nice
         */
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#fundCommitAmount').number(true);

            });</script>";


        $invForm .= "<input type=hidden value=$id name='add'>";



        $invForm .= "<table style='border-bottom: 0px solid blue' width='1000px'><thead>
        <tr>
            <th id='fundAddDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='fundAddFundHeader' valign=top width='10%'>Fund<span style='color:red'>*</span></th>
            <th id='fundAddKindHeader' valign=top width='10%'>Kind<span style='color:black'>*</span></th>
            <th id='fundAddCommitHeader' valign=top width='10%'>Commitment<span style='color:black'>*</span></th>
         </tr></thead>";

        $notes = '';


        $invForm .= "<tbody>";
        $invForm .= "<tr><td valign=top><input size='11%' type=text id='datepickerFundAdd' name='date' value=''></td>";


        /*
         * Fund
         */

        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_cos = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#fund").select2({
                placeholder: "Select a Fund",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach (array_keys($funds) as $id) {
            $fu = mysqli_escape_string($conn, $funds[$id]);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$id, text:'$fu'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#fund").select2("data", preload_data_cos);';

        $invForm .= '});
             </script>';
        $invForm .= '<input type="hidden" id="fund" name="fund" style="width:300px"/><br/>
                 <span style="font-size:10px;text-transform:none">For a new fund just enter its name</span>
           </td>';

        $selectedKind = "vd";
        $invForm .= '<td valign=top><select id="fundKind" name="kind">';
        foreach ($GLOBALS["fundKinds"] as $t) {
            $selected = ($selectedKind == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';

        $invForm .= "<td valign=top style='text-align: left'><input size='20' type=text id='fundCommitAmount'  name='fundCommitAmount'><br/>
                     <input id='hasCarry' type='checkbox' name='hasCarry'> I have carry in this fund
                        </td>";
        $invForm .= "
        <tr><td style='font-family: Verdana,Arial;font-size:12px;font-weight: bold'>NOTES: </td><td colspan=5 valign=top><textarea rows=5 cols=120 id='fundNotes' name='notes'>$notes</textarea></td></tr></table>";


        $invForm .= '<a href="javascript:onclick=$(\'#invWiringDetails\').css(\'display\',\'block\');$(\'#enterEquityText\').css(\'display\',\'none\');$(\'#enterWiringText\').css(\'display\',\'none\');">
                <span id="enterWiringText">Click to Enter Wiring Details</span></a>';
        $invForm .="<div id='invWiringDetails' style='display: none'>

        <table style='border-bottom: 2px solid blue; padding: 15px;' width='1000px'><thead>
       <tr>
           <th id='' valign=top width='10%'></th>
           <th id='' valign=top width='10%'>Bank Name</th>
           <th id='' valign=top width='10%'>Bank Address</th>
           <th id='' valign=top width='10%'>Routing No.</th>
           <th id='' valign=top width='10%'>SWIFT Code</th>
           <th id='' valign=top width='10%'>Acct No.</th>
        </tr></thead>";
        $invForm .= "
            <tr><td style='font-family: Verdana,Arial;font-size:12px;font-weight: bold'>Wire Info: </td>
            <td valign=top><input size='15%' type=text id='wireBankName'  name='wireBankName' value=''></td>";

        $invForm .= '
             <td><input size="20%" type="text" id="wireBankAddress" name="wireBankAddress"></td>';

        $invForm .= "
            <td valign=top><input size='10%' type=text id='wireBankRouting'  name='wireBankRouting' value=''></td>
            <td valign=top><input size='15%' type=text id='wireBankSWIFT'  name='wireBankSWIFT' value=''></td>


            <td valign=top><input size='20%' type=text id='wireBankAcct'  name='wireBankAcct' value=''></td>
            </tr><tr>
            <td style='text-transform:uppercase;font-weight:bold' colspan='2'>Further Credit Acct Name:</td> <td colspan='1' valign=top><input size='20%' type=text id='wireFCName'  name='wireFCName' value=''></td>

            <td style='text-transform:uppercase;font-weight:bold' colspan='2'>Further Credit Acct No.:</td><td colspan='1' valign=top><input size='20%' type=text id='wireFCAcct'  name='wireFCAcct' value=''></td>
            </tr></table></div>";




        $invForm .= "<input type='hidden' id='fundUpdate' name='fundUpdate' value='0'>";
        $invForm .= "<input type='hidden' id='fundAction' name='action' value='addFund'>";
        $invForm .= '<p></p><button type="button" id="newFundInvestmentButton" onclick="addOrUpdateFundCommit()">Add New Fund</button></form>';
        $invForm .= '</div>';   // angelinput

        return $invForm;

    }  /* newFundInvestForm */

    /*
     * return new fund payout form for input person ($id) filled in for investment being edited
     */
    function newFundPayoutForm($id, $conn) {

    $selectedType = "cash";
     /*
      * Create funds map (name, id)
      */
        $funds = array();
        angelGetMyFunds($funds,$id);
        //angeldie(serialize($funds));
        $invForm = "";



        $invForm .= "<form id='fundaddPayoutForm' action='/investments.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";
        $invForm .= "<input type=hidden value=$id name='id'>";
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#fundpayoutCash').number(true);
                $('#fundpayoutShares').number(true);
            });</script>";


        $invForm .= "<input type=hidden value=$id name='add'>";

        $invForm .= '<div class="angelinput">';

        $invForm .= "<table width='1000px'><thead>
        <tr>
            <th id='fundpayoutDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='fundpayoutFundHeader' valign=top width='10%'>Fund<span style='color:red'>*</span></th>
            <th id='fundpayoutTypeHeader' valign=top width='10%'>Type of Dist<span style='color:red'>*</span></th>
            <th id='fundpayoutAmountHeader' valign=top width='10%'>Amount<span style='color:red'>*</span></th>
            <th id='fundpayoutPriceHeader' valign=top width='10%' style='display:none'>Price of Record<span style='color:red'>*</span></th>
            <th id='fundpayoutSymbolHeader' valign=top width='10%' style='display:none'>Symbol</th>
            <th id='fundpayoutNotesHeader' valign=top width='20%'>Notes
         </tr></thead>";

        $notes = '';

        $invForm .= "<tbody>";
        $invForm .= "<tr><td valign=top><input size='11%' type=text id='datepickerPayfund' name='date' value=''></td>";

        /*
         * Fund
         */

        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_cos = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#payoutfund").select2({
                placeholder: "Select a Fund",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach ($funds as $fu) {
            $fundID = $fu["id"];
            $fundName = mysqli_escape_string($conn, $fu["name"]);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$fundID, text:'$fundName'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#payoutfund").select2("data", preload_data_cos);';

        $invForm .= '});
            </script>';
        $invForm .= '<input type="hidden" onchange="payoutAddTaxInfo(0,\'fund\')" id="payoutfund" name="fund" style="width:300px"/><br/>
                     <input type="checkbox" name="lastPayout"> I think this will be the last distribution.
           </td>';

        /*
         * Type
         */

        $invForm .= '<td valign=top><select id="fundpayoutType" name="type">';
        foreach ($GLOBALS["payoutTypes"] as $t) {
            $selected = ($selectedType == $t ) ? 'selected' : '';

            $invForm .= "<option $selected value='$t'>$t</option>";
        }
        $invForm .= '</select></td>';

        $invForm .= "<script>$('#fundpayoutType').bind('change', function(event) {

            var i= $('#fundpayoutType').val();
            // console.debug('change!: ' + i);
            if(i =='stock') {
                // console.debug('payouts - here');
                $('#fundpayoutAmountHeader').html('No. Shares<span style=\"color:red\">*</span>');
                $('#fundpayoutPriceHeader').css('display','table-cell');
                $('#fundpayoutSymbolHeader').css('display','table-cell');
                $('#fundpayoutPriceCell').css('display','table-cell');
                $('#fundpayoutSharesCell').css('display','table-cell');
                $('#fundpayoutSymbolCell').css('display','table-cell');

                $('#fundpayoutCashCell').css('display','none');
            } else {
                $('#fundpayoutAmountHeader').html('Amount<span style=\"color:red\">*</span>');
                $('#fundpayoutPriceHeader').css('display','none');
                $('#fundpayoutSymbolHeader').css('display','none');
                $('#fundpayoutPriceCell').css('display','none');
                $('#fundpayoutSharesCell').css('display','none');
                $('#fundpayoutSymbolCell').css('display','none');

                $('#fundpayoutCashCell').css('display','table-cell');

            }

          });</script>";


        $invForm .= "<td  id='fundpayoutCashCell' valign=top><input size='10%' type=text id='fundpayoutCash'  name='payoutCash' value=''></td>";

        $invForm .= "<td id='fundpayoutSharesCell' valign=top style='display:none'><input size='10%' type=text id='fundpayoutShares'  name='payoutShares' value=''></td>";
        $invForm .= "<td id='fundpayoutPriceCell' valign=top style='display:none'><input size='10%' type=text id='fundpayoutPrice'  name='payoutPrice' value=''></td>";
        $invForm .= "<td id='fundpayoutSymbolCell' valign=top style='display:none'><input size='10%' type=text id='fundpayoutSymbol'  name='payoutSymbol' value=''></td>";


        // <td valign=top><input size='10%' type=checkbox id='lastPayout'  name='lastPayout' value=''></td>";
        $invForm .= "<td valign=top><textarea rows=5 cols=30 id='fundpayoutNotes' name='payoutNotes'>$notes</textarea></td></tr>";
        $invForm .= "<tr><td colspan=7><div id='fundpayoutTaxInfoDiv'></div></td></tr>";
        $invForm .= "</table>";


        $invForm .= "<input type='hidden' id='fundpayoutUpdate' name='update' value='0'>";
        $invForm .= "<input type='hidden' id='fundpayoutAction' name='action' value='addPayout'>";
        $invForm .= '<p></p><button type="button" id="newPayoutButtonfund" onclick="addOrUpdatePayout(\'fund\')">Add New Distribution</button></form>';

        $invForm .= "</div>";   // angelinput div

        return $invForm;

    }  /* newFundPayoutForm */

    /*
      * return new fund payout form for input person ($id) filled in for investment being edited
      */
    function newFundPaymentForm($id, $conn) {

        /*
         * Create funds map (name, id)
         */
        $funds = array();
        angelGetMyFunds($funds,$id);
        //angeldie(serialize($funds));
        $invForm = "";



        $invForm .= "<form id='fundaddPaymentForm' action='/investments.php' method='post'>";
        $invForm .= "<input type=hidden value=$id name='investorID'>";
        $invForm .= "<input type=hidden value=$id name='id'>";
        $invForm .= "<script type='text/javascript'>$(function() {
                $('#fundpaymentAmount').number(true);
            });</script>";


        $invForm .= "<input type=hidden value=$id name='add'>";

        $invForm .= '<div class="angelinput">';

        $invForm .= "<table width='850px'><thead>
        <tr>
            <th id='fundpaymentDateHeader' valign=top width='10%'>Date<span style='color:red'>*</span></th>
            <th id='fundpaymentFundHeader' valign=top width='10%'>Fund<span style='color:red'>*</span></th>
            <th id='fundpaymentAmountHeader' valign=top width='10%'>Amount<span style='color:red'>*</span></th>
            <th id='fundpaymentNotesHeader' valign=top width='20%'>Notes
         </tr></thead>";

        $notes = '';

        $invForm .= "<tbody>";
        $invForm .= "<tr><td valign=top><input size='11%' type=text id='datepickerFundPayment' name='date' value=''></td>";

        /*
         * Fund
         */

        $invForm .= "<td valign=top width='15%'>
            <script>";
        $invForm .= "preload_data_cos = 0";
        $invForm .= '
        $(document).ready(function () {
            $("#paymentfund").select2({
                placeholder: "Select a Fund",
                createSearchChoice:function(term, data) { if ($(data).filter(function() { return this.text.localeCompare(term)===0; }).length===0) {return {id:term, text:term};} },
                allowClear:true,
                multiple: false,';


        $needComma = false;

        $invForm .= "data: [";
        foreach ($funds as $fu) {
            $fundID = $fu["id"];
            $fundName = mysqli_escape_string($conn, $fu["name"]);
            if ($needComma)
                $invForm .= ",";

            $invForm .= "{id:$fundID, text:'$fundName'}";

            $needComma = true;
        }
        $invForm .= "]";


        $invForm .= '});
            $("#paymentfund").select2("data", preload_data_cos);';

        $invForm .= '});
            </script>';
        $invForm .= '<input type="hidden"  id="paymentfund" name="fund" style="width:300px"/><p>
                <input type="checkbox" name="showWireEmail" value="Y" checked> Show wire email?

           </td>';


        $invForm .= "<td   valign=top><input size='10%' type=text id='fundpaymentAmount'  name='paymentAmount' value=''></td>";

        $invForm .= "<td valign=top><textarea rows=5 cols=30 id='fundpaymentNotes' name='paymentNotes'>$notes</textarea></td></tr>";
        $invForm .= "</table>";


        $invForm .= "<input type='hidden' id='fundpaymentUpdate' name='paymentUpdate' value='0'>";
        $invForm .= "<input type='hidden' id='fundpaymentAction' name='action' value='addFundPayment'>";
        $invForm .= '<p></p><button type="button" id="newFundPaymentButton" onclick="addOrUpdateFundPayment()">Add New Payment</button></form>';

        return $invForm;

    }  /* newFundPaymentForm */


    /****************************************************************************
     *  view of Investments by each investor.
     */
    function investorInvestmentsPageBody() {

        $invInvestmentsMap = array();
        $angelMap = array();
        $vcMap = array();


        $int = new ik12Investment();
        $conn = $int->connect();
        $path = $GLOBALS["path"];
        /*
         * Header (IK12 [cohort chooser] Investments
         */

        $this->getInvestorInvestmentMap($conn, $invInvestmentsMap);        // maps investor id to a comma separated list of Investment ids
        ik12_getInvestorMap($angelMap, $conn, 'angel');
        ik12_getInvestorMap($vcMap, $conn, 'vc');

        $body = "<span style='font-size: larger'>Investments (view by <a href='$path/invests'>Company</a>) </span><p></p>";
        $body .= " <div id='container' style='border:0px solid black; float:left; min-height: 500px'>";


        /***********************************************************************************************/
        /*
         *  Right side table: investors with no Investments
         */

        $body .= "<div id='navbar'  style='width: 360px;border: 0px solid blue;overflow: auto;
                                                           max-height: 300px; position: fixed; top:170px;right:40px;'>";
        $body .=        $this->showAllInvestorsWithoutInvestments($conn, $invInvestmentsMap, $angelMap, $vcMap);
        $body .= "</div>";

        // return $body;

        /***********************************************************************************************/
        /*
         * Now comes the left side content: investors with Investments
         */

        $body .= "<div id='content' class='angelinput' style='border:2px solid black;'>";
        $body .=        $this->showAllInvestorsWithInvestments($conn, $invInvestmentsMap, $angelMap, $vcMap);
        $body .= "
                  </div>
                </div>

                ";

        return $body;
    }
    /*
    * Display all of the investors with no Investments.
    */
    function showAllInvestorsWithoutInvestments($conn, $invInvestmentsMap, $angelMap, $vcMap) {

        /*
         * Angels
         */
        $investors = "<div style='float:left; width: 150px;border: 2px solid green;overflow: auto;
                                                           max-height: 300px;'>";

        $investors .= "<table style='border:0px solid black; font-size: 100%' width='100%'>
                        <tr><td style='color:blue;padding-right:5px' align='center' colspan='1'>Angels w/no Investments<hr></td> </tr>";

        foreach (array_keys($angelMap) as $angel) {
            if (!array_key_exists($angel, $invInvestmentsMap))
                $investors .= "<tr><td>$angelMap[$angel]</td></tr>";
        }
        $investors .= "</table></div>";

        /*
         * VCs
         */

        $investors .= "<div style=' float:right;width: 150px;border: 2px solid green;overflow: auto;
                                                           max-height: 300px;'>";

        $investors .= "<table style='border:0px solid black; font-size: 100%' width='100%'>
                        <tr><td style='color:blue;padding-right:5px' align='center' colspan='1'>VCs w/no Investments<hr></td> </tr>";

        foreach (array_keys($vcMap) as $vc) {
            if (!array_key_exists($vc, $invInvestmentsMap))
                $investors .= "<tr><td>$vcMap[$vc]</td></tr>";
        }



        $investors .= "</table></div>";

        return($investors);
    }

    /*
     * Display all of the investors with Investments.
     */
    function showAllInvestorsWithInvestments($conn, $invInvestmentsMap, $angelMap, $vcMap) {

        $path = $GLOBALS["path"];

        $investors = "<table id='InvestmentCohortTable'><thead><th width='10%'>Angel</th><th>Investments</th></thead>";
        // angeldie(serialize($invInvestmentsMap));     // debug
        foreach (array_keys($angelMap)  as $angel) {
            if (array_key_exists($angel, $invInvestmentsMap)) {
                $iRows = $this->InvestmentTableRows($conn, $invInvestmentsMap[$angel]);
                $investors .= "<tr><td><a href='$path/investorpage.php?type=angel&id=$angel'>$angelMap[$angel]</a></td><td><table id='InvestmentTable'>$iRows</table></td>";
            }

        }
        $investors .= "</table>";

        $investors .= "<table id='InvestmentCohortTable'><thead><th width='10%'>VC</th><th>Investments</th></thead>";
        foreach (array_keys($vcMap)  as $vc) {
            if (array_key_exists($vc, $invInvestmentsMap)) {
                $iRows = $this->InvestmentTableRows($conn, $invInvestmentsMap[$vc]);
                $investors .= "<tr><td><a href='$path/investorpage.php?type=investco&id=$vc'>$vcMap[$vc]</a></td><td><table id='InvestmentTable'>$iRows</table></td>";

            }

        }
        $investors .= "</table>";
        /* use arsort() and the below to also create a sorted list of investor by Investment count.
                * substr_count( $_SESSION['sarray'], ",") +1;
                */

        return ($investors);

    }
    /*
     * returns table rows for input comma separated list of Investments
     */
    function InvestmentTableRows($conn, $Investments) {

        $rows = "";
        $Investment = new ik12Investment();
        $Investment->shareconn($conn);

        $co = new angelCompany();
        $co->shareconn($conn);

        $path = $GLOBALS["path"];




        foreach (explode(",", $Investments)  as $id) {
            $row = $Investment->getRows("investmenttable", "id=$id");
            $int = mysqli_fetch_object($row, "ik12Investment");

            $co->id = $int->company_id;
            $co->name = "";

            $co = $co->findMatchingCompany();
            if ($co->cohort != "")
                $name = "<a href='$path/invests?add=$co->id'>$co->name</a>";
            else
                $name = $co->name;

            $amount = sprintf("$%01.2fmm", $int->amount_raised/1000000);

            $rows .= "<tr style='font-size:smaller'>
                          <td width='25%'>$int->inv_date</td>
                          <td style='color:black'>$name</td><td>$int->round</td>
                          <td  style='font-size:smaller'>$amount</td></tr>";
        }

        return($rows);
    }

    /*
     * create map of investor to string of Investments made to them
     */
    function getInvestorInvestmentMap($conn, &$map) {

        $q = "SELECT * from investmenttable";

        $r = mysqli_query($conn, $q);
        if (!$r)
            angeldie("Investment->getInvestorInvestmentMap select Investmenttable Failed: " . mysqli_error($conn));

        /*
         * Create map
         */
        while($int = mysqli_fetch_object($r,"ik12Investment")){
            $id = $int->id;
            if ($int->angel_ids != "") {
                foreach (explode(",", $int->angel_ids)  as $inv) {
                    if (array_key_exists($inv,$map))
                        $map[$inv] .= ",$id";
                    else
                        $map[$inv] = "$id";
                }
                foreach (explode(",", $int->investco_ids)  as $inv) {
                    if (array_key_exists($inv,$map))
                        $map[$inv] .= ",$id";
                    else
                        $map[$inv] = "$id";
                }

            }
        }

    }



function inputError($msg) {

    echo "<span style='color:red'>--> $msg</span><br>";


}

/*
 * this function only exists in case we ever want to resurrect the investment feed
 */
function unusedInvestmentFeed($int) {

    $path = $GLOBALS["path"];

    $feed = "
    <div id='navbar' class='ik12feed' style='width: 360px;border: 2px solid blue; float:right;'>
                    <table style='border:0px solid black; font-size: 100%' width='100%'>
                        <tr><td style='color:blue;padding-right:5px' align='center' colspan='4'>Investment Feed<hr></td> </tr>";

        $query = "SELECT * from angelinvests WHERE investor_id=$investorID ORDER BY inv_date DESC LIMIT 20";
        //angeldie($query);    // debug
        $r = mysqli_query($conn, $query);
        if (!$r)
            angeldie("Investment->showCompany select angelinvests (1) Failed: " . mysqli_error($conn));
        while($int = mysqli_fetch_object($r,"ik12Investment")){
            if ($int->round == 'Acquisition')
                $investorList = $int->acquirer;
            else
                $investorList = ik12_getInvestorList($conn, $int->investco_ids, $int->angel_ids, false);
            $co = new angelCompany();
            $conn = $co->connect();
            $co->id = $int->company_id;

            $co = $co->findMatchingCompany();
            if ($co->cohort != "")
                $name = "<a href='$path/invests?add=$co->id'>$co->name</a>";
            else
                $name = $co->name;
            if ($int->type == "safe" or $int->type == "note")
                $value = sprintf("$%01.2fmm(cap)", $int->cap/1000000);
            else
                $value = sprintf("$%01.2fmm(val)", $int->prevaluation/1000000);

            $amount = sprintf("$%sk", $int->invest_amount);

            $body .=  "<tr style='font-size:smaller'><td width='25%'>$int->inv_date</td><td>$amount</td><td>$value</td><td>$name</td><td  style='font-size:smaller'>$investorList</td></tr>";
        }
        $feed .= "
                    </table>
                    </div>";

    return($feed);
}

}

