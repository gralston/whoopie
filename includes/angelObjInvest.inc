<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 */
 
class angelInvest extends angeldb {

    public $company_id      = 0;
    public $investor_id     = 0;
    public $inv_date        = "";       // investment date
    public $exit_date       = "";       // shutdown, sale, or ipo date
    public $conv_date       = "";       // date at which this investment converted if it was a convertible
    public $round           = "";       //
    public $type            = "";       // safe, note, equity
    public $invest_amount   = 0;        // amount invested
    public $prevaluation   = 0;         // pre money valuation
    public $shares          = 0;        // total number of shares
    public $commonShares    = 0;        // common shares from this investment
    public $prefShares      = 0;        // preferred shares from this investment
    public $prefSharesSeries = "";       // "Series A", "Series B", etc.
    public $commCertNum     = "";       // certificate number for common (e.g. CS-05)
    public $prefCertNum     = "";       // certificate number for preferred (e.g. PA-05)
    public $certLoc         = "";       // location of the certs ("file", "eshares")
    public $converted       = 0;        // when true a convertible investment has converted to shares
    public $proRata         = 0;        // boolean for whether you have pro rata rights
    public $infoRights      = 0;        // boolean for whether you have information rights
    public $qsb             = 0;        // boolean for whether this is a qualified small business
    public $price_paid      = 0.0;      // price paid per share (float)
    public $cap             = 0;        // cap if safe or note in
    public $discount        = 0;        // discount if any for safe or note in percent

    public $investco_ids    = "";       // other investors if you know them
    public $angel_ids       = "";
    public $status          = "";       // "active", "acquired", "ipo", "dead", "prospect"
    public $modelID         = 0;        // if a convert, this may be a model of the conversion
    public $notes           = "";       // whatever you like
    public $YCbatch         = "";       // yc batch indicator for this company, e.g. w2017
    public $basis           = 0;        // query string showing basis used once distributions start. for tax use.







    function __construct() {
        parent::__construct();

        $this->column_names = "company_id,investor_id,inv_date,conv_date,round,type,invest_amount,prevaluation,shares,prefShares,commonShares,
                                prefCertNum,prefSharesSeries,commCertNum,certLoc,converted,proRata,infoRights,qsb,price_paid,cap,discount,
                                investco_ids,angel_ids,status,modelID,notes,YCbatch,basis";
        $this->table_name = "angelinvests";
    }

/*
 * Print out investments for company with id $id
 */
    function showInvestments() {

        /*
         * first order of business, map $id to a name for display purposes
         */
        $id = $this->company_id;
        $con = $this->conn;

        $query = "SELECT name FROM companytable WHERE id = '$id'";

        $result = mysqli_query($con,$query);
        if ($result == NULL)
            angeldie("company query failed");

        $row = mysqli_fetch_array($result,MYSQLI_ASSOC);
        if ($row == NULL)
            angeldie("there is no company with id: $id");

        $name = $row['name'];

        echo "<h3 style='text-align:center'>Investments in <a href='/companypage.php?id=$id'>$name </a></h3><p></p>";

        /*
         * Now get each investment and HTMLize it.
         */
        $query = "SELECT * FROM investmenttable WHERE company_id = '$id' ORDER BY inv_date DESC";

        $result = mysqli_query($con,$query);
        if ($result == NULL)
            angeldie("investment query failed");

        echo '<div class="ik12stats">';

        echo "<table style='width: 100%; border-spacing: 5px; border-collapse: separate'><thead style='text-align: center'>
        <tr><th style='text-align: center;vertical-align:top;width:8%;'>Round</th>
            <th style='text-align: center;vertical-align:top;width:8%;'>Type</th>
            <th style='text-align: center;vertical-align:top;width:10%;'>Date</th>
            <th style='text-align: center;vertical-align:top;width:8%;'>Amount (mm)</th>
            <th style='text-align: center;vertical-align:top;width:12%;'>Post Money Val/Cap (mm)</th>
            <th style='text-align: center;vertical-align:top;width:28%;'>Investors
            <th style='text-align: center;vertical-align:top;width:16%;'>Comments</th>
            <th style='text-align: center;vertical-align:top;width:10%;'>Articles</th></tr></thead>";
        echo "<tbody>";

        while($inv = mysqli_fetch_object($result,"ik12investment")){
            $raised = sprintf("$%01.2f", $inv->amount_raised/1000);
            if ($inv->postmoney_val != 0)
                $value = sprintf("$%01.2f", $inv->postmoney_val/1000);
            elseif ($inv->cap != 0)
                $value = sprintf("$%01.2f", $inv->cap/1000);
            else
                $value = "unknown";

            $investorList = ik12_getInvestorList($con, $inv->investco_ids, $inv->angel_ids, true);
            echo "
        <tr><td valign=top><a id='edit' href='/invests?edit=$inv->id&companyID=$id'>$inv->round</a></td>
            <td valign=top>$inv->type</td>
            <td valign=top>$inv->inv_date</td>
            <td valign=top>$raised</td>
            <td valign=top>$value</td>
            <td valign=top>$investorList</td>
            <td valign=top>$inv->comments</td>
            <td valign=top>$inv->articles</td></tr>";
        }
        echo "</tbody></table></div>";

    }  //showInvestments


    function showInvestmentsView() {

        /*
         * first order of business, map $id to a name for display purposes
         */
        $invests = "";

        $id = $this->company_id;
        $con = $this->conn;
        $cohort = $this->cohort;

        $query = "SELECT name FROM companytable WHERE id = '$id'";

        $result = mysqli_query($con,$query);
        if ($result == NULL)
            angeldie("company query failed");

        $row = mysqli_fetch_array($result,MYSQLI_ASSOC);
        if ($row == NULL)
            angeldie("there is no company with id: $id");

        $name = $row['name'];

        $invests .= "<h3 style='text-align:center'><a href='/invests.php?cohort=$cohort'>Investments</a> in <a href='/companypage.php?id=$id'>$name </a></h3><p></p>";

        /*
         * Now get each investment and HTMLize it.
         */
        $query = "SELECT * FROM investmenttable WHERE company_id = '$id' ORDER BY inv_date DESC";

        $result = mysqli_query($con,$query);
        if ($result == NULL)
            angeldie("investment query failed");

        $invests .= '<div class="ik12stats">';

        $invests .= "<table style='width: 100%; border-spacing: 5px; border-collapse: separate'><thead style='text-align: center'>
        <tr><th style='text-align: center;vertical-align:top;width:8%;'>Round</th>
            <th style='text-align: center;vertical-align:top;width:8%;'>Type</th>
            <th style='text-align: center;vertical-align:top;width:10%;'>Date</th>
            <th style='text-align: center;vertical-align:top;width:8%;'>Amount (mm)</th>
            <th style='text-align: center;vertical-align:top;width:12%;'>Post Money Val/Cap (mm)</th>
            <th style='text-align: center;vertical-align:top;width:28%;'>Investors
            <th style='text-align: center;vertical-align:top;width:16%;'>Comments</th>
            <th style='text-align: center;vertical-align:top;width:10%;'>Articles</th></tr></thead>";
        $invests .= "<tbody>";

        while($inv = mysqli_fetch_object($result,"ik12investment")){
            $raised = sprintf("$%01.2f", $inv->amount_raised/1000);
            if ($inv->postmoney_val != 0)
                $value = sprintf("$%01.2f", $inv->postmoney_val/1000);
            elseif ($inv->cap != 0)
                $value = sprintf("$%01.2f", $inv->cap/1000);
            else
                $value = "unknown";
            if ($inv->round == 'Acquisition')
                $investorList = $inv->acquirer;
            else
                $investorList = ik12_getInvestorList($con, $inv->investco_ids, $inv->angel_ids, true);
            $invests .= "
        <tr><td valign=top><a id='edit' href='/invests?edit=$inv->id&companyID=$id'>$inv->round</a></td>
            <td valign=top>$inv->type</td>
            <td valign=top>$inv->inv_date</td>
            <td valign=top>$raised</td>
            <td valign=top>$value</td>
            <td valign=top>$investorList</td>
            <td valign=top>$inv->comments</td>
            <td valign=top>$inv->articles</td></tr>";
        }
        $invests .= "</tbody></table></div>";

        return $invests;


    }  //showInvestmentsView



  function showInvestments2() {

        /*
         * first order of business, map $id to a name for display purposes
         */
        $id = $this->company_id;
        $con = $this->conn;

        $query = "SELECT name FROM companytable WHERE id = '$id'";

        $result = mysqli_query($con,$query);
        if ($result == NULL)
            angeldie("company query failed");

        $row = mysqli_fetch_array($result,MYSQLI_ASSOC);
        if ($row == NULL)
            angeldie("there is no company with id: $id");

        $name = $row['name'];

        /*
         * Now get each investment and HTMLize it.
         */
        $query = "SELECT * FROM investmenttable WHERE company_id = '$id' ORDER BY inv_date DESC";

        $result = mysqli_query($con,$query);
        if ($result == NULL)
            angeldie("investment query failed");
		$n = mysqli_num_rows($result);

        echo "<table id='ik12stats2'>
        <tr><th style='width:6%;'>Edit</th>
			<th style='width:8%;'>Round</th>
            <th style='width:6%;'>Type</th>
            <th style='width:8%;'>Date</th>
            <th style='width:7%;'>Amount (mm)</th>
            <th style='width:12%;'>Post Money Val/Cap (mm)</th>
            <th style='width:33%;'>Investors</th></tr>";

       $altcolor = 0;
        while($inv = mysqli_fetch_object($result,"ik12investment")){
            $raised = sprintf("$%01.2f", $inv->amount_raised/1000);
            if ($inv->postmoney_val != 0)
                $value = sprintf("$%01.2f", $inv->postmoney_val/1000);
            elseif ($inv->cap != 0)
                $value = sprintf("$%01.2f", $inv->cap/1000);
            else
                $value = "unknown";

            $investorList = ik12_getInvestorList($con, $inv->investco_ids, $inv->angel_ids, true);
			$investorList = strip_tags($investorList);

            if ($altcolor == '0') {echo "<tr class='alt'>";$altcolor = 1; } else {echo "<tr>";$altcolor = 0; }
			
			echo "<td valign=top align=center><a id='edit' href='/dbm/verify_investment.php?edit=$inv->id'><img src='/img/edit-icon.png' width=30></a></td>";
			
			if ($inv->round == 'Seed') {echo "<td valign=top>$inv->round *</td>";} else  {echo "<td valign=top>$inv->round</td>";}
            $date = print_date($inv->inv_date);
		   
			echo "
			<td valign=top>$inv->type</td>
            <td valign=top>$date</td>
            <td valign=top>$raised</td>
            <td valign=top>$value</td>
            <td valign=top>$investorList</td></tr>";
        }
		if ($n == 0) {echo "<td colspan=7 style='text-align:center;height: 40px;'><span style='font-size:16px';>Currently, we have no invesments on record. Please <a href='/dbm/verify_investment.php?new=1'>add a new investment</a></span></td>";}
        echo "</table>";

    }  //showInvestments2

    /*
     * return the total amount invested by the input investor in the input company. If the investorID is 0, return the total amount invested in the company
     */
    function totalInvestment($co, $investorID) {

        $total = 0;
        if ($investorID)        // count investments in this company by this investor
            $where = "company_id=$co->id AND investor_id=$investorID";
        else                    // count all investments by any investor
            $where = "company_id=$co->id";

        //angeldie($where);
        $r = $this->getRows("angelinvests", $where);
        // angeldie(serialize($r));
        while($inv = mysqli_fetch_object($r,"angelInvest")){
            $total += $inv->invest_amount;
        }
        return($total);

    }

    /*
     * A matching investment is one with the same id, the same date, the same company, the same investor
     * and the same amount
     */
    function findMatchingInvest() {

        $where = "company_id=$this->company_id AND inv_date='$this->inv_date' AND investor_id=$this->investor_id AND invest_amount=$this->invest_amount";
        //angeldie($where);    // debug
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
     * return an array with all the investments in the input company by the input investor
     */
    function getInvestments($companyID, $investorID) {

        $invests = array();

        $r = $this->getRows($this->table_name, "company_id=$companyID and investor_id=$investorID");
        while($inv = mysqli_fetch_object($r,"angelInvest")) {
            $invests[] = $inv;
        }

        return($invests);
    }


}  // investment