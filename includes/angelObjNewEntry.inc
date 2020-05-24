<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/11/14
 * Time: 10:59 AM
 */

class angelNewEntry extends angeldb {

    public $type            = "";         // table name of entry
    public $table_id        = 0;        // ID within table of entry
    public $userID          = 0;        // user ID of current user or -1 for guest


    function __construct() {
        parent::__construct();

        $this->column_names = "type,table_id,userID";
        $this->table_name = "new_entries";

    }

    function recentEntries($count, $userID) {
        if ($this->conn == NULL)
            $this->connect();

        $q = "SELECT * from $this->table_name WHERE userID=$userID ORDER BY id  DESC LIMIT $count";
        // angeldie($q);
        $r = mysqli_query($this->conn, $q);

        return($r);

    }

    function oneDayEntries($day) {
        if ($this->conn == NULL)
            $this->connect();

        $q = "SELECT * from $this->table_name WHERE date BETWEEN '$day' AND '$day 23:59:59' ORDER BY id DESC";
        // angeldie($q);    // debug
        $r = mysqli_query($this->conn, $q);

        return($r);

    }

    function searchEntries($str) {
        if ($this->conn == NULL)
            $this->connect();

        $where = sprintf(" WHERE (type LIKE '%%%s%%' )", $str,$str);
        $q = "SELECT * from $this->table_name $where ORDER BY id DESC";
        // angeldie($q);    // debug
        $r = mysqli_query($this->conn, $q);

        return($r);

    }
    function entryTypeToText() {
        if ($this->type == "angelinvests")
            $type = "investment";
        elseif ($this->type == "payouttable")
            $type = "angel dist.";
        elseif ($this->type == "fundpayouttable")
            $type = "fund dist.";
        elseif ($this->type == "fundnotestable")
            $type = "fund note";
        elseif ($this->type == "notestable")
            $type = "co note";
        elseif ($this->type == "converttable")
            $type = "conv. saved";
        elseif ($this->type == "modeltable")
            $type = "model saved";
        elseif ($this->type == "companytable")
            $type = "new company";
        elseif ($this->type == "fundinvests")
            $type = "cap call";
        elseif ($this->type == "fundtable")
            $type = "fund commit";
        elseif ($this->type == "persontable")
            $type = "registration ";
        elseif ($this->type == "fundmodeltable")
            $type = "fund model";
        else
            $type = $this->type;

        return($type);

    }
    /*
     * This function is big and unwieldy :).  it returns a text string representing short (at most maxInfoSize chars)
     * but useful abbreviation of what the input entry was all about.
     *
     */

    function entryToInfo() {

        $maxInfoSize = 52;
        $companyColor = "blue";
        $fundColor = "red";

        if ($this->type == "angelinvests") {
            $inv = new angelInvest();
            $inv->id = $this->table_id;

            $inv = $inv->getEntry();
            if ($inv == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");
            $co = new angelCompany();
            $co->id = $inv->company_id;
            $co = $co->findMatchingCompany();
            if ($co != null)
                 $coName = $co->name;
            else
                $coName = "<unknown>";
            $amount = number_format($inv->invest_amount);

            $info = "\$$amount in <span style='color:$companyColor'>$coName</span>";

        } elseif ($this->type == "payouttable") {
            $pay = new angelPayout();
            $pay->id = $this->table_id;

            $pay = $pay->getEntry();
            if ($pay == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");

            $co = new angelCompany();
            $co->id = $pay->companyID;

            $co = $co->findMatchingCompany();

            if ($co != NULL)
                $coName = $co->name;
            else
                $coName = "[unknown]";

            if ($pay->type == "cash")
                $amount = "$" . number_format($pay->cash);
            else {
                $shares = number_format($pay->shares);
                $amount = "$shares of $pay->symbol at \$$pay->sharePrice";
            }

            $info = "$amount from <span style='color:$companyColor'>$coName</span>";

        } elseif ($this->type == "fundpayouttable") {

            $pay = new angelPayout();
            $pay->id = $this->table_id;
            $pay->settableName("fundpayouttable");

            $pay = $pay->getEntry();



            $co = new angelFund();
            $co->id = $pay->companyID;

            $co = $co->findMatchingFund(true);
            //angeldie(serialize($this). "|||" . serialize($pay));

            if ($co != NULL)
                $coName = $co->name;
            else
                $coName = "[unknown]";

            if ($pay->type == "cash")
                $amount = "$" . number_format($pay->cash);
            else {
                $shares = number_format($pay->shares);
                $amount = "$shares of $pay->symbol at \$$pay->sharePrice";
            }

            $info = "$amount from <span style='color:$fundColor'>$coName</span>";

        } elseif ($this->type == "fundnotestable") {
            $n = new angelFundNote();
            $n->id = $this->table_id;

            $n = $n->getEntry();
            if ($n == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");

            $co = new angelFund();
            $co->id = $n->fundID;

            $co = $co->findMatchingFund(true);

            if ($co != NULL)
                $coName = $co->name;
            else
                $coName = "[unknown]";

            $type = $n->type;
            $notes = $n->notes;

            $info = "for <span style='color:$fundColor'>$coName</span> | $type | <i>$notes</i> ";
        } elseif ($this->type == "notestable") {
            $n = new angelNote();
            $n->id = $this->table_id;

            $n = $n->getEntry();
            if ($n == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");

            $co = new angelCompany();
            $co->id = $n->companyID;

            $co = $co->findMatchingCompany();

            if ($co != NULL)
                $coName = $co->name;
            else
                $coName = "[unknown]";

            $type = $n->type;
            $notes = $n->notes;

            $info = "for <span style='color:$companyColor'>$coName</span> | $type | <i>$notes</i> ";
        } elseif ($this->type == "converttable") {
            $n = new angelConvert();
            $n->id = $this->table_id;

            $n = $n->getEntry();
            if ($n == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");

            $type = $n->type;
            $cap = "$" . number_format($n->cap);

            $info = "$type with cap $cap";

        } elseif ($this->type == "modeltable") {
            $n = new angelModel();
            $n->id = $this->table_id;

            $n = $n->getEntry();
            if ($n == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");

            $info = "with $n->numberFounders founder(s) and $n->numberConverts convert(s) ";
        } elseif ($this->type == "companytable")
            $info = "new company";
        elseif ($this->type == "fundinvests") {
            $inv = new angelFundInvest();
            $inv->id = $this->table_id;

            $inv = $inv->getEntry();
            if ($inv == null)
                return("no entry found for id '" . $this->table_id . "' -- probably deleted.");
            $co = new angelFund();
            $co->id = $inv->fundID;
            $co = $co->findMatchingFund(true);
            if ($co != null)
                $coName = $co->name;
            else
                $coName = "<unknown>";
            $amount = number_format($inv->investAmount);

            $info = "\$$amount to fund <span style='color:$fundColor'>$coName</span>";

        } elseif ($this->type == "fundtable")
            $info = "fund commit";
        elseif ($this->type == "persontable")
            $info = "registration ";
        elseif ($this->type == "fundmodeltable")
            $info = "fund model";
        else
            $info = $this->type;

        if (strlen(strip_tags($info)) > $maxInfoSize) {
            $info = substr($info, 0, $maxInfoSize);
            $info .= "...";
        }

        return($info);
    }


} 