<?php
/**
 * Created by Geoff.
 * User: geoff
 * Date: 6/6/2020
 * Time: 2:32 PM
 */
class whoopieGame extends angeldb {


    public $creator             = "";    // name of person who created this game
    public $creationdate        = "";   // date game created
    public $name                = "";   // game name
    public $description         = "";   // game description
    public $playercount         = 0;    // number of players expected
    public $players             = 0;    // number of players currently
    public $playernames         = "";   // playerid:name, playerid:nane, 
    public $finalscore          = "";    // playerid:score, playerid;scoer
    public $winner              = 0;    // playerid who won  
    public $status              = "";   // current status: wait, active, gameover


    

    function __construct() {
        parent::__construct();
        $this->table_name = "games";
        $this->column_names = 'creator,creationdate,name,description, playercount,players,playernames,finalscore,winner,status';
    }

    function findMatchingGame($exact) {

        if ($this->conn == null)
            $this->connect();

        $rows = NULL;
        if ($this->name != "") {
            $name = mysqli_escape_string($this->conn, $this->name);
            // echo("name is: " . $name);  // debug
            if ($exact)
                $rows = $this->getRows($this->table_name, "name = '$name'");
            else
                $rows = $this->getRows($this->table_name, "name LIKE '$name%'");
        } elseif ($this->id != 0) {
            $id =   $this->id;
            $rows = $this->getRows($this->table_name, "id=$id");
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
            $game = mysqli_fetch_object($rows,"whoopieGame");
            // angeldie("uh oh: $this->name");
            return $game;
        }

        angeldie("findmatchinggame: uh oh - id: $this->id");
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