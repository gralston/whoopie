<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/2/14
 * Time: 1:38 PM
 */
class angelCompany extends angeldb {

    public $creatorID       = 0;    // id of investor who added this company
    public $kind            = "";   // defined as: 'startup', 'corporation', 'vc', ...
    public $name            = "";   // company name
    public $other_names     = "";   // former or other corporate names
    public $email           = "";
    public $logo            = "";   // link to logo image
    public $description     = "";   // description
    public $shortdesc       = "";   // brief description
    public $YCbatch         = "";   // YC batch if a YC company
    public $YCID            = "";   // for future use, internal ID of YC company
    public $sector          = "";   // healthcare, edtech, etc.
    public $location        = "";   // current physical location
    public $url             = "";
    public $founder_ids     = "";   // ids of each founder
    public $status          = "";   // current company status: active, dead, acquired, or ipo
    public $acquirer        = "";    // if acquired, name of company
    public $acquiPrice      = 0;    // if acquired acquisition price
    public $FDshares        = 0;    // fully diluted shares outstanding
    public $valuation       = 0;    // last known company valuation
    public $sharePrice      = 0;    // last known share price
    public $yr_founded      = "";
    public $exitDate        = "";    // acquisition, ipo, or death date
    public $qsb             = false; // true if this is a qualified small business
    public $articles        = "";
    public $tags            = "";
//    public $company_aliases = "";


    function __construct() {
        parent::__construct();
        $this->table_name = "companytable";
        $this->column_names = 'creatorID,kind,name,other_names,email,logo,description,shortdesc,YCbatch,YCID,sector,location,url,founder_ids,status,acquirer,acquiPrice,FDshares,valuation,sharePrice,yr_founded,exitDate,qsb,articles,tags';
    }

    function findMatchingCompany() {

        if ($this->conn == null)
            $this->connect();

        $rows = NULL;
        if ($this->name != "") {
            $name = mysqli_escape_string($this->conn, $this->name);
            // echo("name is: " . $name);  // debug
            $rows = $this->getRows("companytable", "name LIKE '$name%'");
        } elseif ($this->id != 0) {
            $id =   $this->id;
            $rows = $this->getRows("companytable", "id=$id");
        }
        if ($rows == NULL)
            $numrows = 0;
        else
            $numrows = mysqli_num_rows($rows);

        if ($numrows == 0) {
            /*
             * no name match, now try other_names (not yet implemented)
             */

        } else if ($numrows == 1) {
            $existingCompany = mysqli_fetch_object($rows,"angelCompany");
            // angeldie("uh oh: $this->name");
            return $existingCompany;
        }

        return NULL;

    }
    function companyName() {

        $id = $this->id;
        $name = "";
        $query = "SELECT name FROM companytable WHERE id = '$id'";

        $result = mysqli_query($this->conn,$query);
        if ($result == NULL)
            ik12die("companyName query failed");

        $row = mysqli_fetch_array($result, MYSQLI_ASSOC);
        if ($row != NULL)
            $name = $row['name'];

        return($name);

    }

    function basisUsed($investorID) {

        $id = $this->id;

        $inv = new angelInvest();
        $conn = $inv->connect();
        $invests = $inv->getInvestments($id, $investorID);
        $total = 0;
        /*
         * for each investment for input company id / investor id, return the total basis used
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


}  // angelcompany

?>