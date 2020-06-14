<?php
/**
 * Created by Geoff.
 * User: geoff
 * Date: 6/6/2020
 * Time: 2:32 PM
 */
class whoopieGamelog extends angeldb {

    public $gameid           = 0;       // the game this is relevant to
    public $playerid         = 0;       // player who initiated the event
    public $playername       = "";      // player's name
    public $name             = "";      // event name
    public $eventcount       = 0; 
    public $dealer           = 0;  
    public $cardplayed       = "";  
    public $bidmade          = "";  
    public $deck             = "";  
     
    

    function __construct() {
        parent::__construct();
        $this->table_name = "gamelog";
        $this->column_names = 'gameid,playerid,playername, name,eventcount, dealer,cardplayed,bidmade,deck';
    }

    function findMatchingFund($exact) {

        if ($this->conn == null)
            $this->connect();

        $rows = NULL;
        if ($this->name != "") {
            $name = mysqli_escape_string($this->conn, $this->name);
            // echo("name is: " . $name);  // debug
            if ($exact)
                $rows = $this->getRows("fundtable", "name = '$name'");
            else
                $rows = $this->getRows("fundtable", "name LIKE '$name%'");
        } elseif ($this->id != 0) {
            $id =   $this->id;
            $rows = $this->getRows("fundtable", "id=$id");
        }
        if ($rows == NULL)
            $numrows = 0;
        else
            $numrows = mysqli_num_rows($rows);

        if ($numrows == 0) {
            /*
             * no name match, now try other_names (not yet implemented)
             */
            return NULL;

        } else if ($numrows == 1) {
            $existingFund = mysqli_fetch_object($rows,"angelFund");
            // angeldie("uh oh: $this->name");
            return $existingFund;
        }

        return -1;      // multiple matches, this is bad!

    }
    function fundName() {

        $id = $this->id;
        $name = "";
        $query = "SELECT name FROM fundtable WHERE id = '$id'";

        $result = mysqli_query($this->conn,$query);
        if ($result == NULL)
            ik12die("fundName query failed");

        $row = mysqli_fetch_array($result, MYSQLI_ASSOC);
        if ($row != NULL)
            $name = $row['name'];

        return($name);

    }
    /*
     * Retrieve this fund's cashflow model as a query string or "" if there is no model
     */
    function getModel() {
        $mod = new angelFundCFModel();
        $mod->connect();

        $row = $mod->getRows("fundmodeltable", "fundID=$this->id");
        if (mysqli_num_rows($row) != 1)
            return("");
        else {
            $foundMod = mysqli_fetch_object($row, "angelFundCFModel");
            return($foundMod->percentPerYear);
        }

    }
    /*
     * Add a fund's cashflow model
     */
    function addModel($model, $investorID) {
        $mod = new angelFundCFModel();
        $mod->connect();

        $mod->creationDate    = date('Y-m-d');       // today
        $mod->fundID = $this->id;
        $mod->investorID = $investorID;
        $mod->percentPerYear = $model;

        $mod->insertRow();
        /*
         * check for errors!
         */

    }

    /*
     * Update this fund's cashflow model
     */
    function updateModel($model) {
        $mod = new angelFundCFModel();
        $mod->connect();

        $mod->updateRow("percentPerYear=$model", "fundID=$this->id");
        /*
         * check for errors!
         */

    }

    /*
     * Remove a fund's cashflow model
     */
    function deleteModel() {
        $mod = new angelFundCFModel();
        $mod->connect();

        $mod->deleteRows("fundmodeltable", "fundID=$this->id");

    }

    function basisUsed($investorID) {

        $id = $this->id;

        $inv = new angelInvest();
        $conn = $inv->connect();
        $invests = $inv->getInvestments($id, $investorID);
        $total = 0;
        /*
         * for each investment for input fund id / investor id, return the total basis used
         */
        // angeldie(serialize($invests));
        foreach ($invests as $inv) {
            $invBasis = new angelBasis();

            $invBasis->queryString = $inv->basis;
            $total += $invBasis->totalBasis();
        }

        return($total);
    }

    function clearBasis($investorID) {

        $id = $this->id;

        $inv = new angelInvest();
        $conn = $inv->connect();
        $invests = $inv->getInvestments($id, $investorID);

        $pay = new angelPayout();
        $conn = $pay->shareconn($conn);
        $payouts = $pay->getPayouts($id, $investorID);

        /*
         * for each investment and payout clear basis used and update
         */
        // angeldie(serialize($invests));
        foreach ($invests as $inv) {
            $inv->basis = "";
            $inv->updateThisRow();
        }

        foreach ($payouts as $pay) {
            $pay->basis = "";
            $pay->updateThisRow();
        }

    }


}  // angelfund

?>