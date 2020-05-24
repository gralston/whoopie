<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 */
 
class angelFundInvest extends angeldb {

    public $fundID          = 0;        // the fund invested in
    public $investorID      = 0;        // the investor doing the investing
    public $invDate         = "";       // date of investment or commitment
    public $type            = "commit"; // 'commit' = commitment or 'invest' = part of committment invested. or "carry"
    public $investAmount    = 0;        // amount invested or committed on this date
    public $hasCarry        = false;    // the investor also has carry in this fund
    public $status          = "";       // "active" or "closed"
    public $notes           = "";       // whatever you like
    public $basis           = 0;        // query string showing basis used once distributions start. for tax use.


    function __construct() {
        parent::__construct();

        $this->column_names = "fundID,investorID,invDate,type,investAmount,hasCarry,status,notes,basis";
        $this->table_name = "fundinvests";
    }



    /*
     * return the total amount invested by the input investor in the input fund. If the investorID is 0, return the total amount invested in the company
     */
    function totalInvestment($fu, $investorID) {

        $total = 0;
        if ($investorID)        // count investments in this company by this investor
            $where = "fundID=$fu->id AND investorID=$investorID AND type='invest'";
        else                    // count all investments by any investor
            $where = "fundID=$fu->id AND type='invest'";

        //angeldie($where);
        $r = $this->getRows("fundinvests", $where);
        // angeldie(serialize($r));
        while($inv = mysqli_fetch_object($r,"angelFundInvest")){
            $total += $inv->investAmount;
        }
        return($total);

    }

    /*
     * A matching investment is one with the same id, the same date, the same fund, the same investor
     * and the same amount
     */
    function findMatchingInvest() {

        $where = "fundID=$this->fundID AND invDate='$this->invDate' AND investorID=$this->investorID AND investAmount=$this->investAmount";
        //angeldie($where);    // debug
        $rows = $this->getRows($this->table_name, $where );

        if ($rows == NULL) {
            // angeldie(mysqli_error($this->returnconn()));     // debug
            $numrows = 0;
        } else
            $numrows = mysqli_num_rows($rows);

        if ($numrows != 1)
            return NULL;


        $existingInv = mysqli_fetch_object($rows,"angelFundInvest");

        return $existingInv;


    }

 /*
  * A matching commitment is one with the same fund, same investor and type of commit
  */
    function findMatchingCommit() {

        $where = "fundID=$this->fundID AND investorID=$this->investorID AND type='commit'";
        //angeldie($where);    // debug
        $rows = $this->getRows($this->table_name, $where );

        if ($rows == NULL) {
            // angeldie(mysqli_error($this->returnconn()));     // debug
            $numrows = 0;
        } else
            $numrows = mysqli_num_rows($rows);

        if ($numrows != 1)
            return NULL;


        $existingInv = mysqli_fetch_object($rows,"angelFundInvest");

        return $existingInv;


    }
    /*
     * return an array with all the investments in the input fund by the input investor
     */
    function getInvestments($fundID, $investorID) {

        $invests = array();

        $r = $this->getRows($this->table_name, "fundID=$fundID and investorID=$investorID");
        while($inv = mysqli_fetch_object($r,"angelFundInvest")) {
            $invests[] = $inv;
        }

        return($invests);
    }


}  // investment