<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/2/14
 * Time: 1:38 PM
 */
class angelFund extends angeldb {

    public $creatorID       = 0;    // id of investor who added this fund
    public $kind            = "";   // defined as: 'vc', 'hedge', ...
    public $name            = "";   // fund name
    public $other_names     = "";   // former or other names
    public $inceptionYear   = "";   // year fund began operations
    public $companyID       = "";   // id of company who is the fund's General Partner
    public $status          = "";   // "active" or "closed"
    public $description     = "";   // description
    public $email           = "";   // email address of fund
    public $address         = "";   // address of fund
    public $url             = "";   // url of fund if there is one
    public $wireBank        = "";   // bank name for wires
    public $wireBankAddress = "";   // Address of bank for wires
    public $wireBankRouting = "";   // 9 digit wiring number
    public $wireBankSWIFT   = "";   // SWIFT code for intl. wires
    public $wireBankAcct    = "";   // account number for wires
    public $wireAcctName    = "";   // official fund name for wires
    public $wireFundAddress = "";   // fund address for wires
    public $wireReference   = "";   // reference if any (e.g. the wire source)
    public $wireFurtherName = "";   // if a for further credit wire, name on account
    public $wireFurtherAcct = "";   // if a for further credit wire, acct number to use
    public $closeDate       = "";   // date fund closed


    function __construct() {
        parent::__construct();
        $this->table_name = "fundtable";
        $this->column_names = 'creatorID,kind,name,other_names,inceptionYear,companyID,status,description,email,address,url,wireBank,wireBankAddress,wireBankRouting,wireBankSWIFT,wireBankAcct,wireAcctName,wireFundAddress, wireReference,wireFurtherName,wireFurtherAcct,closeDate';
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