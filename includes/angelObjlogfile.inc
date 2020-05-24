<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/11/14
 * Time: 10:59 AM
 */

class angelLogfile extends angeldb {

    public $date            = "";
    public $script          = "";       // php program who is writing the log entry
    public $ip              = "";       // ip of user
    public $user            = "";       // login/email of user
    public $descripton      = "";       // log message / event
    public $userID          = 0;        // userID of logged in user or 0 if a guest
    public $userHistory     = "";       // if empty this is a system log, otherwise it is a defined user history event
    public $refer           = "";       // referral URL

    function __construct() {
        parent::__construct();

        $this->column_names = "date,script,ip,user,description,userID,userHistory,refer";
        $this->table_name = "logtable";

    }

    function recentLogs($count) {
        if ($this->conn == NULL)
            $this->connect();

        $q = "SELECT * from $this->table_name ORDER BY id DESC LIMIT $count";
        $r = mysqli_query($this->conn, $q);

        return($r);

    }

    function oneDayLogs($day) {
        if ($this->conn == NULL)
            $this->connect();

        $q = "SELECT * from $this->table_name WHERE date BETWEEN '$day' AND '$day 23:59:59' ORDER BY id DESC";
        // angeldie($q);    // debug
        $r = mysqli_query($this->conn, $q);

        return($r);

    }

    function searchLogs($str) {
        if ($this->conn == NULL)
            $this->connect();

        $where = sprintf(" WHERE (description LIKE '%%%s%%' or user LIKE '%%%s%%')", $str,$str);
        $q = "SELECT * from $this->table_name $where ORDER BY id DESC";
        // angeldie($q);    // debug
        $r = mysqli_query($this->conn, $q);

        return($r);

    }



    function logStats() {

        if ($this->conn == NULL)
            $this->connect();

        $logStatNames = array (
            "Login Page Requests" => array("login page request", 0,0,0),
            "Failed Logins" => array("login failed", 0,0,0),
            "User Logins" => array("successful login", 0,0,0),
            "Root Logins" => array("ROOT login", 0,0,0),
            "Logouts" => array("logging out", 0,0,0),
            "Reg Req" => array("registration request", 0,0,0),
            "New Regs" => array("successful new registration", 0,0,0),
            "Bad Regs" => array("registration failed", 0,0,0),
            "Regs/OW" => array("successful overwrite registration", 0,0,0),
            "JS Invest Reqs" => array("ajax investment request",0,0,0),
            "Logged In Model" => array("logged in model page", 0,0,0),
            "Guest Compare" => array("guest compare page", 0,0,0),
            "Logged In Compare" => array("logged in compare page", 0,0,0),
            "investments.php" => array("Investment request", 0,0,0),
            "Angel Invests" => array("add investment request", 0,0,0),
            "Dists: angel+fund" => array("add payout request", 0,0,0),
            "Fund Commits" => array("add fund request", 0,0,0),
            "Cap Calls" => array("add fund payment request", 0,0,0),

            "Ptable" => array("People table accessed", 0,0,0),
            "Ctable" => array("Company table accessed", 0,0,0),
            "Mtable" => array("Model table accessed", 0,0,0),
            "Guest Model" => array("guest model page", 0,0,0)
        );

        $day = date('Y-m-d', strtotime("today"));

        $q = "SELECT * from $this->table_name WHERE date BETWEEN '$day' AND '$day 23:59:59'";
        $r = mysqli_query($this->conn, $q);
        if (!$r)
           angeldie("logfile access (logStats-1) failed.");

        while($l = mysqli_fetch_object($r,"angelLogfile")){
            foreach(array_keys($logStatNames) as $key) {
                if (strpos(strtolower($l->description), strtolower($logStatNames[$key][0])) !== false)
                    $logStatNames[$key][1]++;
            }

        }

        $weekAgoDay = date('Y-m-d', strtotime("-1 week"));

        $q = "SELECT * from $this->table_name WHERE date BETWEEN '$weekAgoDay' AND '$day 23:59:59'";
        // angeldie($q); // debug
        $r = mysqli_query($this->conn, $q);
        if (!$r)
            angeldie("logfile access (logStats-2) failed.");
        while($l = mysqli_fetch_object($r,"angelLogfile")){
            foreach(array_keys($logStatNames) as $key) {
                if (strpos(strtolower($l->description), strtolower($logStatNames[$key][0])) !== false)
                    $logStatNames[$key][2]++;
            }

        }

        $monthAgoDay = date('Y-m-d', strtotime("-1 month"));

        $q = "SELECT * from $this->table_name WHERE date BETWEEN '$monthAgoDay' AND '$day 23:59:59'";
        // angeldie($q); // debug
        $r = mysqli_query($this->conn, $q);
        if (!$r)
            angeldie("logfile access (logStats-2) failed.");
        while($l = mysqli_fetch_object($r,"angelLogfile")){
            foreach(array_keys($logStatNames) as $key) {
                if (strpos(strtolower($l->description), strtolower($logStatNames[$key][0])) !== false)
                    $logStatNames[$key][3]++;
            }

        }

        return($logStatNames);
    }


} 