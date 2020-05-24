<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 */
 
class angelPayout extends angeldb {

    public $companyID      = 0;         // investment in this company led to the payout
    public $investorID     = 0;
    public $payDate         = "";       // payment date
    public $type            = "";       // cash or stock
    public $cash            = 0;        // payout in dollars if cash
    public $shares          = 0;        // number of shares
    public $symbol          = "";       // symbol for shares received (add name too?)
    public $sharePrice      = 0;        // price of record for shares (at deal). If unknown, 0.
    public $shareLoc        = "";       // location of the shares ("file", "eshares")
    public $shareType       = "";       // common or preferred
    public $basis           = "";       // basis to apply to payout: investmentID1=$ amount to apply&investmentID2=$ amount to apply
    public $lastPayout      = false;    // true if this is (believed to be) the last payout
    public $longterm        = true;     // true if this is a long term investment
    public $notes           = "";       // probably useful to have!

    function __construct() {
        parent::__construct();

        $this->column_names = "companyID,investorID,payDate,type,cash,shares,symbol,sharePrice,shareLoc,shareType,basis,lastPayout,longterm,notes";
        $this->table_name = "payoutTable";
    }


 /*
  * return the total amount returned by the input investor in the input company. If the investorID is 0, return the total amount returned by the company
  */
    function totalReturn($co, $investorID) {

        $total = 0;
        if ($investorID)        // count returns from this company to this investor
            $where = "companyID=$co->id AND investorID=$investorID";
        else                    // count all returned to any investor
            $where = "companyID=$co->id";

        //angeldie($where);
        $r = $this->getRows("payoutTable", $where);
        // angeldie(serialize($r));
        while($po = mysqli_fetch_object($r,"angelPayout")){
            if ($po->type == "cash")
                $total += $po->cash;
            else
                $total += $po->shares * $po->sharePrice;
        }
        return($total);

    }

    /*
     * return the total amount returned by the input investor in the input fund. If the investorID is 0, return the total amount returned by the fund
     * recall that the table stores the fundID as in the companyID column
     */
    function totalFundReturn($fund, $investorID) {

        $total = 0;
        if ($investorID)        // count returns from this company to this investor
            $where = "companyID=$fund->id AND investorID=$investorID";
        else                    // count all returned to any investor
            $where = "companyID=$fund->id";

        // angeldie($where);
        $r = $this->getRows("fundpayouttable", $where);
        // angeldie(serialize($r));
        while($po = mysqli_fetch_object($r,"angelPayout")){
            if ($po->type == "cash")
                $total += $po->cash;
            else
                $total += $po->shares * $po->sharePrice;
        }
        return($total);

    }

    /*
      * A matching investment is one with the same id, the same date, the same company, the same investor
      * and the same amount
      */
    function findMatchingPayout() {

        $where = "companyID=$this->companyID AND payDate='$this->payDate'
                    AND investorID=$this->investorID AND type='$this->type'
                    AND cash='$this->cash' AND shares='$this->shares'";
        // angeldie($where);    // debug
        $rows = $this->getRows($this->table_name, $where );

        if ($rows == NULL) {
            // angeldie(mysqli_error($this->returnconn()));     // debug
            $numrows = 0;
        } else
            $numrows = mysqli_num_rows($rows);

        if ($numrows != 1)
            return NULL;


        $existingInv = mysqli_fetch_object($rows,"angelInvest");

        return $existingInv;


    }

    /*
  * return an array with all the payouts in the input company by the input investor
  */
    function getPayouts($companyID, $investorID) {

        $payouts = array();

        $r = $this->getRows($this->table_name, "companyID=$companyID and investorID=$investorID");
        while($pay = mysqli_fetch_object($r,"angelPayout")) {
            $payouts[] = $pay;
        }

        return($payouts);
    }

}  // payout