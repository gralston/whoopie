<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 *    object for a convert which is part of a model
 */
 
class angelConvert extends angeldb {

    public $model_id        = 0;
    public $type            = "";       // "SAFE" or "Note" or "YCVC SAFE" or "Post SAFE" or "Custom" or "Manual"
    public $yourInvestment   = 0;       // amount you invested in this convert, if any
    public $totalInvestment   = 0;      // total amount invested in this convert
    public $cap              = 0;        // cap
    public $discount        = 0;        // discount in percent
    public $custom          = "";       // comma separated attributes for a custom convertible
    public $preexist        = "";       // comma separated indices of preexisting converts to include in denominator
    public $shares         = 0;         // for a manual entry the actual total shares this convertible received


    function __construct() {
        parent::__construct();

        //$this->column_names = "company_id,investor_id,inv_date,round,type,invest_amount,postvaluation,shares,certNum,certLoc,price_share,price_paid,cap,discount,investco_ids,angel_ids,total_fdshares,current_valuation,status,notes,convert_id";
        $this->column_names = "model_id,type,yourInvestment,totalInvestment,cap,discount,custom,preexist,shares";
        $this->table_name = "converttable";
    }



}  // investment