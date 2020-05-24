<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 */
 
class angelFundPayout extends angelPayout {

    public $fundID      = 0;         // investment in this fund led to the payout
    public $gainLTPercent = 0;       // Percent of payout which is a LT gain
    public $gainSTPercent = 0;       // Percent of payout which is a ST gain
    public $QSB;

    function __construct() {
        parent::__construct();

        $this->column_names = "fundID,investorID,payDate,type,cash,shares,symbol,sharePrice,shareLoc,shareType,basis,lastPayout,longterm,notes";
        $this->table_name = "fundpayoutTable";
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
            if ($po->type == "Cash")
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