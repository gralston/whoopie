<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 5/26/14
 * Time: 10:23 PM
 */

/*
 * Global Variables
 */
date_default_timezone_set("America/Los_Angeles");

$GLOBALS["bookfaceCohort"] = "f2015";       // don't switch bookface until we actually start the new cohort
$GLOBALS["cohort"] = "f2015";
$GLOBALS["nextCohort"] = "f2016";
$GLOBALS["currentYear"] = $currentYear = date("Y");
$GLOBALS["cohortTable"] = "f2014_apps";      // this should be the single place this is used
$GLOBALS["allCohorts"] = array('f2015','f2014','f2013','w2013','s2012','w2012','s2011');
$GLOBALS["investmentRounds"] = array('seed','series seed','series A','series AA', 'series B','series C','series D','bridge');
$GLOBALS["investmentTypes"] = array('note','safe','equity','warrant','option','other');
$GLOBALS["trackerTypes"] = array('company','person');
$GLOBALS["trackerFrequencies"] = array('monthly','quarterly','yearly');
$GLOBALS["trackerStarts"] = array('Now','Next Month','Next Quarter');
$GLOBALS["fundInvestmentTypes"] = array('commit','invest');
$GLOBALS["fundKinds"] = array('vc','hedge','lbo','growth equity','real estate','debt','other');
$GLOBALS["payoutTypes"] = array('cash','stock');
$GLOBALS["companyNoteTypes"] = array('meeting','update','office hours','misc','valuation update','exit prediction','stock split');
$GLOBALS["investorRatings"] = array(5,4,3,2,1,"No Rating");
$GLOBALS["conn"] = null;                    // if non-null we are connected to the db
$GLOBALS["loggedInUser"] = "";                // logged in user if any
$GLOBALS["loggedInUserFname"] = "";           // first name of logged in user
$GLOBALS["loggedInUserColName"] = "";         // name to be used in column references for IK12 users only
$GLOBALS["loggedInUserID"] = -1;               // user id of logged in user
$GLOBALS["loggedInUserCompanyID"] = 0;        // company id of logged in user
$GLOBALS["logo"] = "img/geoffangel3.png";
$GLOBALS["logo"] = "img/angelcalc_logo_long.png";
$GLOBALS["path"] = "";  //  this was needed when angel was under htdocs
$GLOBALS["betaFile"] = "betaList.txt";            // file containing beta users
$GLOBALS["guestID"] = -1;

/*
 *  Log $msg to the logging table (logtable) and include the current user
 *  create table if not exists logtable(id int(8) not null auto_increment primary key, date timestamp,script varchar(128),
 * ip varchar(16), user varchar(32), description varchar(128),refer varchar(128));
 *
 * make sure not to call ik12die, since he will call angellog which could get into some nasty infinite recursion
 */
function ik12log($msg) {
    angellog($msg);
}

function angellog($msg) {

    if (isset($_SESSION) and array_key_exists('logname', $_SESSION) and $_SESSION['logname'] != "") {
        $user   = $_SESSION['logname'];
    } else
        $user   = '[not logged in]';

    // die("angellog: (user, session): " . angelParens($user, $_SESSION['logname']));

    $script = $_SERVER['PHP_SELF'];
    $today  = date("Y-m-d H:i:s");
    if (array_key_exists('HTTP_REFERER', $_SERVER))
        $refer  = $_SERVER['HTTP_REFERER'];
    else
        $refer  = 'none';
    $ip     = $_SERVER['REMOTE_ADDR'];

    $logTableDefinition = "
        id	                int(8) NOT NULL AUTO_INCREMENT PRIMARY KEY,
        date	            timestamp,
        script  	        varchar(128),
        ip  	            varchar(16),
        user	            varchar(128),
        description	        text,
        refer       	    varchar(128)
";


    // ik12die("logtest");
    $db = new angeldb();
    $conn = $db->connect();



    /*
     * Check to see if logtable exists, if it doesn't create it
     */
    $sql = "SELECT * FROM information_schema.tables WHERE TABLE_NAME = 'logtable';";
    $r = mysqli_query($conn, $sql);
    if (!$r)
        die("log failed - probably your database password is incorrect: " . mysqli_error($conn));

    if (mysqli_num_rows($r) == 0) {
        /*
         * no log table - create it.
         */
        $sql="CREATE TABLE  logtable(" . $logTableDefinition . ") ROW_FORMAT=DYNAMIC ENGINE=MyISAM;";
        if (!mysqli_query($conn, $sql)) {
            die("failed creating logtable: " . mysqli_error($conn));;
        }

    }
    /*
     * Log table exists - now log it!
     */


    $msg = mysqli_escape_string($conn, $msg);   // make sure there string has not killer quotes in it!
    $sql = "INSERT INTO logtable (date, script, ip, user,description,refer)
                   VALUES ('$today','$script','$ip','$user','$msg','$refer')";
    mysqli_query($conn, $sql) or die("Can't execute logging query: " . $sql . "error: " . mysqli_error($conn));

}


/*
 *  Print out an error message and then die
 */
function angeldie($msg) {

    angellog('angeldie called with: ' . $msg);
    echo "<table id='deathTable'>";
    echo "<tr><td>";
    echo "<span style='color:blue'>Internal Angel Error: </span>";
    echo $msg;
    echo "</td></tr>";
    echo "<tr><td>";
    echo "<span style='color:red'>Please send an email to: support@angelcalc.com with the error message above.</span>";
    echo "</td></tr>";
    echo "<tr><td>";
    echo "<span style='color:red'>Also include the name of the page you were on and what you were doing at the time of the error.</span>";
    echo "</td></tr>";
    echo "</table>";
    die("");

}

function ik12die($msg) {

    angeldie($msg);

}

/*
 *  Print out an error message and then die - but in an appropriate way for a user
 */
function angelerror($msg) {

    angellog('angel error called with: ' . $msg);
    $v = new angelview();

    $body = "<table id='angelOfDeathTable'>";
    $body .= "<tr><td><span style='color:blue; font-weight:700; font-size:20px;'>Error</span></td></tr>";

    $body .= "<tr><td>$msg</td></tr>" ;
    $body .= "<tr><td>";
    $body .= "<span style='color:black; font-weight:100; font-size:12px;'>Please try again (usually the back button will work).</span>";
    $body .= "</td></tr>";
    $body .= "</table>";

    $v->outputHTML($body);
    die("");

}

/*
 *  Print out an error message and then die - but in an appropriate way for a user
 */
function ik12error($msg) {

 angelerror($msg);

}
function isRoot($investorID) {

    if ($investorID == 0 or $_SESSION["auth"] == "root")
        return(true);
    else
        return(false);
}

/*
 * the input investor has edit access to $co if they are root, or if the company has open access (no investor set)
 * or if the investor added the company.
 */
function angelHasEditRights($investorID, $creatorID) {
    // return(false);   // for debug
    $hasRights = false;

    if (isRoot($investorID))
        $hasRights = true;
    elseif ($creatorID == 0)
        $hasRights = true;
    elseif ($creatorID == $investorID)     // need to change $co->investorID to $co->accessList (list of investors with access)
        $hasRights = true;

    return($hasRights);

}

/*
 * useful function to see if a number is in a list of numbers
 */
function angelNumberIsInList($no, $list) {
    $arr = explode(",", $list);
    /*
     * okay - need to write this!
     */


}

function angelIsLoggedIn() {
    session_start();
    if (!array_key_exists('auth', $_SESSION) or !array_key_exists("isAngel", $_SESSION)) {
        return(false);
    } else {

        $sessionAuth = $_SESSION['auth'];
        $isAngel = $_SESSION['isAngel'];

        if ($sessionAuth == "" or $sessionAuth == "no" or !$isAngel) {
           return(false);
        } else
            return(true);
    }
}

/*
 *
 */
function angelAuthenticate($levelRequired) {

    session_start();
    if (!array_key_exists('auth', $_SESSION) or !array_key_exists("isAngel", $_SESSION)) {
        header("Location: "  . $GLOBALS["path"] . "/login");                // kick them out to login
        exit();
    }


    $sessionAuth = $_SESSION['auth'];
    $isAngel = $_SESSION['isAngel'];

    // ik12die($sessionAuth);   // debug


    if ($sessionAuth == "" or $sessionAuth == "no" or !$isAngel) {
        angellog("Rejected access w/out login");
        header("Location: "  . $GLOBALS["path"] . "/login");                // kick them out to login
        exit();
    }

    if ($sessionAuth == "root")
        return "root";

    if ($levelRequired == "")
        $levelRequired = "root";

    if ($levelRequired == "root" and $sessionAuth != "root") {
        if ($sessionAuth == "ik12founder") {
            angellog("Rejected access root/ik12founder");
            header("Location: "  . $GLOBALS["path"] . "/model");                // kick them out to bookface
            exit();
        } else {
            angellog("Rejected access root/$sessionAuth (?)");
            header("Location: "  . $GLOBALS["path"] . "/login");;                // kick them out to login - really noone here but
            exit();
        }                                                   // educators and they aren't supported yet.
    } else if ($levelRequired == "ik12founder" and $sessionAuth != "ik12founder") {
        if ($sessionAuth == "founder") {
            angellog("Rejected access ik12founder/founder");
            header("Location: /status");                // kick them out to bookface
            exit();
		}
	    if ($sessionAuth == "tir") {
	        angellog("Rejected access ik12founder/tir");
	        header("Location: /tir.php");                // kick them out to bookface
	        exit();
        } else {
            angellog("Rejected access ik12founder/?");
            header("Location: "  . $GLOBALS["path"] . "/login");                // kick them out to login
            exit();
        }
    }

    return $sessionAuth;
}

/*
 * Deprecated!
 *
function ik12AuthBasic() {
    session_start();
    $auth = @$_SESSION['auth'];
    if ($auth != "yes" and $auth != 'ik12founder') {
        $uri = $_SERVER['REQUEST_URI'];
        $uri = urlencode($uri);
        header("Location: /login.php?from=$uri");
        // echo("Location: /login.php?from='$uri'");
        exit();
    }

    return $auth;
}
*/

function angelAuthUser($p) {
    return(ik12AuthUser($p));
}

function ik12AuthUser($p) {
    if ($p->kind == "ik12person")
        return "root";
    else
        return $p->kind;

}

class SimpleClass
{
    // property declaration
    public $var = 'a default value';

    // method declaration
    public function displayVar() {
        echo $this->var;
    }
}

class appTableRows
{
    // property declaration
    public $numRows = 0;
    public $rows = '';
    public $cohort = '';
    public $conn;
    public $whichApps;
    public $submittedDate = "<td><i>submitted</i> &nbsp; ";   // setting this to "" eliminates this from output
    public $communicatedDate = "<td><i>communicated</i> &nbsp; ";   // setting this to "" eliminates this from output

    function __construct() {
        $this->cohort = $GLOBALS["cohort"];
        $this->submittedDate = "<td><i>submitted</i> &nbsp; ";
    }

    /*
     * create table rows as a string in this->rows and the number of rows in this->numRows
     * note that $interview, $Reference, $company_name, etc. come magically from EXTRACT [not anymore - update
     * this comment assuming we keep the changes]
     */


    public function createAppRows() {

        $altcolor = 0;   
        $query = "SELECT * FROM apptable WHERE cohort = '$this->cohort' AND status = 'Complete' ORDER BY finish_time DESC";
        $result = mysqli_query($this->conn, $query) or ik12die("couldn't execute " . $this->whichApps + "query");

       // while ($row = mysqli_fetch_array($result,MYSQL_ASSOC))
        $i = 0;
        while($app = mysqli_fetch_object($result,"ik12app")) {
            // echo "$i<p>";
            $i++;

            //EXTRACT($row);
            $needRow = false;
            if ($app->start_time == "Date") {   // I don't know why this row exists, maybe the header row?  Anyway, skip it.
                continue;
            }
            if ($this->submittedDate != '') {
                if ($app->finish_time != "")
                    $d = print_month_day_year($app->finish_time);
                else
                    $d = "no date";

                $this->submittedDate = "<td><i>submitted</i> &nbsp; " . $d . "</td>";

            }

            if ($this->communicatedDate != '') {
                if ($app->comm_date != "0000-00-00" || $app->comm_date != "")
                    $c = print_month_day_year($app->comm_date);
                else
                    $c = "no date";
                $this->communicatedDate = "<td><i>communicated</i> &nbsp; " . $c . "</td>";

            }
            $col = $_SESSION["loggedInUserColname"] . '_decision';
            switch ($this->whichApps) {
                case "YourReview":
                    $dec = $app->{$col} == "";
                     if ($app->decision == "" && $dec)
                        $needRow = True;
                    break;
                case "OtherReview":
                    $dec = $app->{$col} != "";
                    if ($app->decision == "" && $dec) { 
	                  if ($app->tim_decision == "" || $app->geoff_decision == "" || $app->karen_decision == "" || $app->x_decision == "") {
                        $needRow = True; }}
                    break;
	           case "Discussion":
	                $needRow = (($app->decision == "" && $app->tim_decision != "" && $app->geoff_decision != "" && $app->karen_decision != ""));
	                break;
               case "Selected":
                    $needRow = ($app->interview == "Selected");
                    break;
	            case "TurnedUsDown":
	                $needRow = ($app->interview == "TurnedUsDown");
	                 break;
                case "Interviews":
                    $needRow = (($app->decision == "Interview" || $app->decision == "Interview (Skype)" || $app->decision == "Request more info") && $app->interview == "");
                    break;

                case "InterviewedRejected":
                    $needRow = (($app->decision == "Interview" || $app->decision == "Interview (Skype)") && $app->interview == "Decline" && $app->comm_date != "");
                    break;
                case "ReviewedRejected":
                    $needRow = ($app->decision == "Decline"  && $app->comm_date != "");
                    break;

            }

            /*
             * Need to fix f2014... here and elsewhere
             */
            if ($needRow) {
				if ($altcolor == '0') {$this->rows .= "<tr class='alt'>"; $altcolor = 1; } else {$this->rows .= "<tr>"; $altcolor = 0;}
                $this->rows .= "<td width='35%'><a href='/apps/app_display.php?reference_id=$app->reference'>$app->company_name</a></td>" . $this->submittedDate . "</td>" . $this->communicatedDate . "</td></tr>";

                $this->numRows += 1;
                // echo "numrows: $this->numRows";
            }

        }
    }
}

/**
 *  Creates an sql string from an associative array - assumed to be inserting/updating a row
 *  $table in the db table name
 *  $array is the array
 *  $insert is optional.  allows a user to use an update id to update is in array
 *  V1.0
 *  Last Updated Oct 4. 2009
 **/
function ik12_arrayToSql($conn, $table, $array, $insert = "INSERT INTO") {

    //Check if user wants to insert or update
    if ($insert == "UPDATE") {

        $needComma = false;
        $updateQ = "";
        foreach ($array as $key => $value) {
            if ($key == "id") {
                $id = $value;
            } else {
                if ($needComma)
                    $updateQ .= ", ";

                $updateQ .= $key . "='" . mysqli_escape_string($conn, $value) . "'";
                $needComma = true;
            }
        }

        $sql =  sprintf("UPDATE %s SET %s WHERE id=%s",$table, $updateQ, $id );

    } else {

        $columns = array();
        $data = array();

        foreach ( $array as $key => $value) {
            $columns[] = $key;
            if ($value != "") {
                $data[] = "'" . mysqli_escape_string($conn, $value)  . "'";
            } else {
                $data[] = "''";
            }

            //TODO: ensure no commas are in the values
        }

        $cols = implode(",",$columns);
        $values = implode(",",$data);

        $sql = sprintf("%s %s (%s) VALUES (%s);", $insert,$table, $cols, $values);

    }
    return $sql;

}    // End  ik12_arrayToSql()

/*
 * return in the input array an associative array of every company this investor has funded in asc. alpha order
 */

function angelGetMyCompanies(&$myCos,$id) {
    $co = new angelCompany();
    /*
     * get only those companies which this investor has invested in - note multiple investments in the same
     * company are counted one time here.
     */
    $q = "SELECT DISTINCT c.* FROM companytable as c LEFT JOIN angelinvests AS a ON c.id = a.company_id WHERE a.investor_id=$id ORDER BY c.name";
    $r = $co->getRowsCustom($q);


    if ($r == NULL) {
        angellog("angelGetMyCompanies: getRowsCustom failed: " . $co->error());
        angeldie("angelGetMyCompanies:getRowsCustom failed: " . angelParens($co->error(), $q));
    }

    while($row = mysqli_fetch_assoc($r)) {
        $myCos[] = $row;
    }
}


/*
 * return in the input array an associative array of every one of the investors active company in asc. alpha order
 */

function angelGetAllMyActiveCompanies(&$activeCos, $id) {
    $co = new angelCompany();
    /*
     * get only those companies which this investor has invested in - note multiple investments in the same
     * company are counted one time here.
     */
    $q = $q = "SELECT DISTINCT c.* FROM companytable as c LEFT JOIN angelinvests AS a ON c.id = a.company_id WHERE a.investor_id=$id and c.status='active' ORDER BY c.name";
    $r = $co->getRowsCustom($q);


    if ($r == NULL) {
        angellog("angelGetAllMyActiveCompanies: getRowsCustom failed: " . $co->error());
        angeldie("angelGetAllMyActiveCompanies:getRowsCustom failed: " . $co->error());
    }

    while($row = mysqli_fetch_assoc($r)) {
        $activeCos[] = $row;
    }
}

/*
* return in the input array an associative array of every fund this investor has funded in asc. alpha order
*/

function angelGetMyFunds(&$myFunds,$id) {
    $fu = new angelFund();
    /*
     * get only those companies which this investor has invested in - note multiple investments in the same
     * company are counted one time here.
     */
    $q = "SELECT DISTINCT f.* FROM fundtable as f LEFT JOIN fundinvests AS i ON f.id = i.fundID WHERE i.investorID=$id ORDER BY f.name";
    $r = $fu->getRowsCustom($q);


    if ($r == NULL) {
        angellog("angelGetMyFunds: getRowsCustom failed: " . $fu->error());
        angeldie("angelGetMyFunds:getRowsCustom failed: " . $fu->error());
    }

    while($row = mysqli_fetch_assoc($r)) {
        $myFunds[] = $row;
    }
}

/*
 * return in the input array a list of all ik12 companies by id (order by alpha name
 * for now, but this could be a param)
 */

function getIK12companies(&$ik12Companies) {

   $c = new ik12company();
    $conn = $c->connect();
    if (!$conn) {
        ik12die($c->error());
    }

    $q = "SELECT id, name from companytable WHERE cohort != '' ORDER BY name ASC";

    $r = mysqli_query($conn, $q);
    if (!$r)
        ik12die("getIK12companies select Failed: " . mysqli_error($conn));

    while($co= mysqli_fetch_object($r,"ik12company")){
        $ik12Companies[$co->id] = $co->name;
    }

}
function ik12_getInvestorMap(&$map, $conn, $kind) {

    if ($kind == 'vc' or $kind == 'startup')
        $query = "SELECT id,name from companytable WHERE kind='$kind' ORDER BY name";
    else
        $query = "SELECT id,firstname,lastname from persontable WHERE kind='angel' ORDER BY lastname";

    // ik12die($query);    // debug
    $result = mysqli_query($conn, $query);
    if (!$result)
        ik12die('getInvestorMap: VC lookup failed: ' . $query . mysqli_error($conn));

    while($row = mysqli_fetch_array($result)) {
        if ($kind == 'vc' or $kind == 'startup')
            $map[$row['id']] = $row['name'];
        else
            $map[$row['id']] = $row['firstname'] . " " . $row['lastname'];
    }
    // ik12die($kind . serialize($map));    // debug
}

function ik12_createPersonMap(&$map, $conn) {

    $query = 'SELECT id,firstname,lastname from persontable ORDER BY lastname';

    $result = mysqli_query($conn, $query);
    if (!$result)
        ik12die('createPersonMap: person lookup failed: ' . $query . mysqli_error($conn));

    while($row = mysqli_fetch_array($result)) {
        $map[$row['id']] = $row['firstname'] . " " . $row['lastname'];
    }
    // ik12die(serialize($map));    // debug
}



function ik12_getTagMap(&$map, $conn) {

    $query = 'SELECT id,name from tagtable WHERE id > 0';

    $result = mysqli_query($conn, $query);
    if (!$result)
        ik12die('getTags: Tagtable lookup failed: ' . $query . mysqli_error($conn));

    while($row = mysqli_fetch_array($result)) {
            $map[$row['id']] = $row['name'];
        }
    // ik12die(serialize($map));    // debug
}

function ik12_getSchoolTagMap(&$map, $conn) {

    $query = 'SELECT id,name from school_tags WHERE id > 0';

    $result = mysqli_query($conn, $query);
    if (!$result)
        ik12die('getSchoolTags: School Tag lookup failed: ' . $query . mysqli_error($conn));

    while($row = mysqli_fetch_array($result)) {
            $map[$row['id']] = $row['name'];
        }
    // ik12die(serialize($map));    // debug
}

function ik12_getSubjectTagMap(&$map, $conn) {

    $query = 'SELECT id,name from subject_tags WHERE id > 0';

    $result = mysqli_query($conn, $query);
    if (!$result)
        ik12die('getSubjectTags: Subject Tag lookup failed: ' . $query . mysqli_error($conn));

    while($row = mysqli_fetch_array($result)) {
            $map[$row['id']] = $row['name'];
        }
    // ik12die(serialize($map));    // debug
}


/**************************/

/*
 * return a comma separated list of investors as identified in VCs and Angels (comma sep lists of ids)
 */
function ik12_getInvestorList($conn, $vcs, $angels, $addlinks) {

    $vcQuery = ik12_selectQuery($conn, 'companytable', 'id,name', $vcs);
    $angelQuery = ik12_selectQuery($conn, 'persontable', 'id,firstname,lastname', $angels);

    $investors = "";
    $needComma = false;

    if ($vcQuery != "") {    // there are VC's
        $result = mysqli_query($conn, $vcQuery);
        if (!$result) {
            ik12die('getInvestorList: VC lookup failed: ' . $vcQuery . mysqli_error($conn));

        }
        while($row = mysqli_fetch_array($result)) {
            if ($needComma)
                $investors .= ", ";
            if ($addlinks) {
                $id = $row["id"];
                $investors .= "<a href=/investorpage.php?type=investco&id=$id>" . $row['name'] . "</a>";
            } else
                $investors .= $row['name'];
            $needComma = true;
        }
    }

    if ($angelQuery != "") {    // there are Angels's
        $result = mysqli_query($conn, $angelQuery);
        if (!$result) {
            ik12die('getInvestorList: Angel lookup failed: ' . $angelQuery . mysqli_error($conn));

        }
        while($row = mysqli_fetch_array($result)) {
            if ($needComma)
                $investors .= ", ";
            if ($addlinks) {
                $id = $row["id"];
                $investors .= "<a href=/investorpage.php?type=angel&id=$id>" . $row['firstname'] . " " . $row['lastname'] . "</a>";
            } else
                $investors .= $row['firstname'] . " " . $row['lastname'];
            $needComma = true;
        }
    }

    return "$investors";
}

/*
 * return a comma separated list of investor ids in idArray some of whom may be new and thus are names
 */

function ik12_getInvestorIDs($conn, $idArray, $type) {


    $idList = "";
    $needComma = false;
    foreach(array_keys($idArray) as $i) {
        if ($needComma)
            $idList .= ",";
        else
            $needComma = true;

        $id = $idArray[$i];     // grab the key

        /*
         * If it is a number, then this is a known investor, just add the id to the list
         * Otherwise this is a new investor. Add them to the person or company database and then
         * get their ID.
         */
        if (is_numeric($id))
            $idList .= $id;
        else {
            $id = mysqli_escape_string($conn, $id);
            if ($type == 'angel') {
                $names = preg_split("/[\s,]+/", $id, 2);
                $firstname = $names[0];
                $lastname = $names[1];
                // ik12die('id: ' . $id . " first :" . $firstname . " last: " . $lastname);

                $q = "INSERT INTO persontable (kind,firstname,lastname) VALUES ('angel','$firstname','$lastname')";
                $result = mysqli_query($conn,$q);
                if ($result == NULL)
                    ik12die("persontable insert of new investor $id failed:" . $q );

                $q = "SELECT id from persontable WHERE firstname='$firstname' AND lastname='$lastname'";
                $result = mysqli_query($conn,$q);

                if ($result == NULL)
                    ik12die("persontable select for new investor $id failed:" . $q );

                $p = mysqli_fetch_object($result, 'ik12person');
                $idList .= $p->id;

            } else {
                // a VC
                // ik12die(serialize($idArray) . " id: " . $id);

                $q = "INSERT INTO companytable (kind,name) VALUES ('vc','$id')";
                $result = mysqli_query($conn,$q);
                if ($result == NULL)
                    ik12die("companytable insert of new investor $id failed:" . $q );

                $q = "SELECT id from companytable WHERE name='$id'";
                $result = mysqli_query($conn,$q);

                if ($result == NULL)
                    ik12die("companytable select for new investor $id failed:" . $q );

                $co = mysqli_fetch_object($result, 'ik12company');
                $idList .= $co->id;


            }


        }

    }

    return $idList;

}



/*
 * return a string which is a list of the investors in company $co (with dups - investors who have invested
 * in multiple rounds removed. note - this is not implemented yet, right now no deduping is done.
 */
function getUniqueInvestors( $conn, $co ) {

    /*
     *
     */
    $vcarr = array();
    $angelarr = array();
    $angels = "";
    $vcs = "";

    $id = $co->id;


    /*
     * Now get all of the investors
     */
    $query = "SELECT * FROM investmenttable WHERE company_id = '$id' ORDER BY inv_date DESC";

    $result = mysqli_query($conn,$query);
    if ($result == NULL)
        ik12die("investment query failed");

    while($inv = mysqli_fetch_object($result,"ik12investment")){
        /*
         $vcarr = array_merge($vcarr, explode($inv->investco_ids,","));
        $angelarr = array_merge($angelarr, explode($inv->angel_ids,","));
        */
        $angels .= $inv->angel_ids;
        $vcs .= $inv->investco_ids;
    }
    $investors = ik12_getInvestorList($conn, $vcs, $angels, false);

    return($investors);
}


/*
 * Return a string intended to be used in a SELECT query with the key being one or more ids.
 */
function ik12_selectQuery($conn, $table, $cols, $ids) {
    if ($ids == "")
        return "";

    $needOR = false;
    $query = "SELECT $cols from $table WHERE ";

    foreach (explode(",", $ids) as $k) {

        if ($needOR) {
            $query .= " OR ";
        }

        $query .= "id LIKE " . $k;
        $needOR = true;
    }

    return $query;

}

/**************************/

/*
 * Take the input (relative) path to a photo and move to a destination after resizing to the
 * desired width (height is proportional)
 */
function angelMakeThumb($src, $dest, $desired_width) {

    if (preg_match('/jpg|jpeg|JPG|JPEG/',$src)){
        $source_image = imagecreatefromjpeg($src);
        $isJPG = true;
    } elseif (preg_match('/png|PNG/',$src)){
        $source_image = imagecreatefrompng($src);
        $isJPG = false;
    } else
        angeldie("Illegal image format: only jpegs and pngs are supported.");

    $width = imagesx($source_image);
    $height = imagesy($source_image);

    /* find the "desired height" of this thumbnail, relative to the desired width  */
    $desired_height = floor($height * ($desired_width / $width));

    /* create a new, "virtual" image */
    $virtual_image = imagecreatetruecolor($desired_width, $desired_height);
    // imagecolortransparent($virtual_image, imagecolorallocate($virtual_image, 0, 0, 0));
    if (!$isJPG) {
        imagealphablending( $virtual_image, false );
        imagesavealpha( $virtual_image, true );
    }

    /* copy source image at a resized size */

    imagecopyresampled($virtual_image, $source_image, 0, 0, 0, 0, $desired_width, $desired_height, $width, $height);


    /* store the physical thumbnail image on disk */
    if ($isJPG)
        imagejpeg($virtual_image, $dest);
    else
        imagepng($virtual_image, $dest);

    /* free up imagine memory */
    imagedestroy($virtual_image);
    imagedestroy($source_image);
}

function angelImageType($image) {
    $imageType = "";


    if (preg_match('/jpg|jpeg|JPG|JPEG/',$image)){
        $imageType = "jpg";
    } elseif (preg_match('/png|PNG/',$image)){
        $imageType = "png";
    }


    return($imageType);
}
/**************************/

/*
 * Send an email with the subject and body from angelcalc's mailbox
 */
function angelSendEmail($subject, $body, $firstname, $email) {
    require_once "Mail.php";

    $to         = $email;                     // Send email to our user
    $from       = "noreply@angelcalc.com";   // this won't do anything since gmail rewrites it
    $host       = "ssl://smtp.gmail.com";
    $port       = "465";
    $username   = 'angelcalc.mailbox@gmail.com';
    $password   = '7times7IS49';


    $greeting   = "Dear $firstname,
    ";

    $body = $greeting . $body;


    $headers = array ('From' => $from, 'To' => $to,'Subject' => $subject);
    $smtp = Mail::factory('smtp',
        array ('host' => $host,
            'port' => $port,
            'auth' => true,
            'username' => $username,
            'password' => $password));

    $mail = $smtp->send($to, $headers, $body);

    if (PEAR::isError($mail)) {
        angeldie($mail->getMessage());
    }

}

/**************************/

/*
 * Send an email with the subject and body from ik12's mailbox
 */
function ik12SendHTMLEmail($subject, $body, $firstname, $email) {
    require_once "Mail.php";

    $to         = $email;                     // Send email to our user
    $from       = "noreply@imaginek12.com";   // this won't do anything since gmail rewrites it
    $host       = "ssl://smtp.gmail.com:465";
    $username   = 'ik12.mailbox@gmail.com';
    $password   = 'edtechk12';

    // ik12die("ik12SendEmail: " . $body);   // debug
	
	$m = '1.0';
	$h = 'text/html; charset=iso-8859-1';
	$headers = array ('MIME-Version' => $m, 'Content-type' => $h, 'From' => $from, 'To' => $to,'Subject' => $subject);
    $smtp = Mail::factory('smtp',
        array ('host' => $host,
            'auth' => true,
            'username' => $username,
            'password' => $password));

    $mail = $smtp->send($to, $headers, $body);

    if (PEAR::isError($mail)) {
        ik12die($mail->getMessage());
    }

}


/**************************/

function angelformatPhoneNumber($phoneNumber) {
    $phoneNumber = preg_replace('/[^0-9]/','',$phoneNumber);

    if(strlen($phoneNumber) > 10) {
        $countryCode = substr($phoneNumber, 0, strlen($phoneNumber)-10);
        $areaCode = substr($phoneNumber, -10, 3);
        $nextThree = substr($phoneNumber, -7, 3);
        $lastFour = substr($phoneNumber, -4, 4);

        $phoneNumber = '+'.$countryCode.' ('.$areaCode.') '.$nextThree.'-'.$lastFour;
    }
    else if(strlen($phoneNumber) == 10) {
        $areaCode = substr($phoneNumber, 0, 3);
        $nextThree = substr($phoneNumber, 3, 3);
        $lastFour = substr($phoneNumber, 6, 4);

        $phoneNumber = '('.$areaCode.') '.$nextThree.'-'.$lastFour;
    }
    else if(strlen($phoneNumber) == 7) {
        $nextThree = substr($phoneNumber, 0, 3);
        $lastFour = substr($phoneNumber, 3, 4);

        $phoneNumber = $nextThree.'-'.$lastFour;
    }

    return $phoneNumber;
}

function print_note_time($dat)
{
    $notime = explode(" ",$dat);
    $parts = explode("-",$notime[0]);
    $year = $parts[0];
    $day = $parts[2];
    switch ($parts[1]) {
        case 1: $month = "January"; $mon = "Jan"; break;
        case 2: $month = "February"; $mon = "Feb"; break;
        case 3: $month = "March"; $mon = "Mar"; break;
        case 4: $month = "April"; $mon = "Apr"; break;
        case 5: $month = "May"; $mon = "May"; break;
        case 6: $month = "June"; $mon = "Jun"; break;
        case 7: $month = "July"; $mon = "Jul"; break;
        case 8: $month = "August"; $mon = "Aug"; break;
        case 9: $month = "September"; $mon = "Sep"; break;
        case 10: $month = "October"; $mon = "Oct"; break;
        case 11: $month = "November"; $mon = "Nov";  break;
        case 12: $month = "December";  $mon = "Dec"; break;
        default:
            $month = "xxx"; break;
    }
    $d = "$mon $day, $year";

    $today = getdate();
    $tparts = explode(":",$notime[1]);
    $hour_difference = $today[hours] - $tparts[0];
    $minute_difference = $today[minutes] - $tparts[1];

    if ($today[month]==$month && $today[mday]==$day && $today[year]==$year)  {
        if ($hour_difference == 0) {return "$minute_difference minutes ago";}
        else {if ($hour_difference == 1) {return "1 hour ago";} else {return "$hour_difference hours ago";}}
    }
    else {return $d;}
}

function print_month_day_year($dat)
{
    $notime = explode(" ",$dat);
    $parts = explode("-",$notime[0]);
    $year = $parts[0];
    $day = $parts[2];
    switch ($parts[1]) {
        case 1: $month = "Jan"; break;
        case 2: $month = "Feb"; break;
        case 3: $month = "Mar"; break;
        case 4: $month = "Apr"; break;
        case 5: $month = "May"; break;
        case 6: $month = "Jun"; break;
        case 7: $month = "Jul"; break;
        case 8: $month = "Aug"; break;
        case 9: $month = "Sep"; break;
        case 10: $month = "Oct"; break;
        case 11: $month = "Nov"; break;
        case 12: $month = "Dec"; break;
        default:
            $month = "xxx"; break;
    }

    $d = "$month $day, $year";
    return $d;
}

// assumes $dat is in GMT time and converts to Pacific Time

function print_day_time_gmt($dat)
{
    $time = explode(" ",$dat);
    $parts = explode("-",$time[0]);
    $year = $parts[0];
    $day = $parts[2];
    switch ($parts[1]) {
        case 1: $month = "Jan"; break;
        case 2: $month = "Feb"; break;
        case 3: $month = "Mar"; break;
        case 4: $month = "Apr"; break;
        case 5: $month = "May"; break;
        case 6: $month = "Jun"; break;
        case 7: $month = "Jul"; break;
        case 8: $month = "Aug"; break;
        case 9: $month = "Sep"; break;
        case 10: $month = "Oct"; break;
        case 11: $month = "Nov"; break;
        case 12: $month = "Dec"; break;
    }
    $d = "$month. $day, $year";

    $tod = "am";
    $t = explode(":",$time[1]);
    $h = $t[0];
    if ($h < 8) {$day = $day - 1; $h = $h + 16 ;}  else { $h = $h - 8;}
    if ($h > 12) {$h=$h-12; $tod ="pm";}
    if ($h == 12) {$tod ="pm";}
    $m = $t[1];

    $p = "$h:$m$tod PT on $d";

    return $p;
}

function print_day_time($dat)
{
    $time = explode(" ",$dat);
    $parts = explode("-",$time[0]);
    $year = $parts[0];
    $day = $parts[2];
    switch ($parts[1]) {
        case 1: $month = "Jan"; break;
        case 2: $month = "Feb"; break;
        case 3: $month = "Mar"; break;
        case 4: $month = "Apr"; break;
        case 5: $month = "May"; break;
        case 6: $month = "Jun"; break;
        case 7: $month = "Jul"; break;
        case 8: $month = "Aug"; break;
        case 9: $month = "Sep"; break;
        case 10: $month = "Oct"; break;
        case 11: $month = "Nov"; break;
        case 12: $month = "Dec"; break;
    }
    $d = "$month. $day, $year";

    $tod = "am";
    $t = explode(":",$time[1]);
    $h = $t[0];
    if ($h > 12) {$h=$h-12; $tod ="pm";}
    $m = $t[1];

    $p = "$h:$m$tod on $d";

    return $p;
}

function print_date($dat)
{
    $parts = explode("-",$dat);
    $year = $parts[0];
    switch ($parts[1]) {
        case 1: $month = "Jan"; break;
        case 2: $month = "Feb"; break;
        case 3: $month = "Mar"; break;
        case 4: $month = "Apr"; break;
        case 5: $month = "May"; break;
        case 6: $month = "Jun"; break;
        case 7: $month = "Jul"; break;
        case 8: $month = "Aug"; break;
        case 9: $month = "Sep"; break;
        case 10: $month = "Oct"; break;
        case 11: $month = "Nov"; break;
        case 12: $month = "Dec"; break;
    }
    $d = "$month $year";
    return $d;
}
function getAngelSectors($conn) {

    $sectors = array();

    $se = new angeldb();

    $se->shareconn($conn);
    $r = $se->getRows("sectorTable", "name != '' ORDER BY name ASC");

    while ($row = mysqli_fetch_row($r)) {
        $sectors[$row[0]] = $row[1];
    }
    return($sectors);

}



function getAngelRounds($conn) {

    $rounds = array();

    $ro = new angeldb();

    $ro->shareconn($conn);
    $r = $ro->getRows("roundTable", "name != '' ORDER BY id ASC");

    while ($row = mysqli_fetch_row($r)) {
        $rounds[$row[0]] = $row[1];
    }
    return($rounds);

}

/*
 * the input round is either an id referring to a round - return the name or
 * a new round in which case write it to the db
 */

function angelRound($round, $conn) {

    if ($round != "" and !is_numeric($round)) {
        /*
         * this is a new round
         *
         */
        $round = mysqli_escape_string($conn, $round );
        $q = "INSERT INTO roundtable (name) VALUES ('$round')";
        $result = mysqli_query($conn,$q);
        if ($result == NULL)
            angeldie("angelRound: roundtable insert of new round $round failed:" . $q );

    } else {
        /*
         * round is the id of the round we want - look up and return the  round name:
         * we decided the indirection was just not worth the trouble.
         */
        $db = new angeldb();
        $db->shareconn($conn);
        if ($round != "") {
            $r = $db->getRows("roundTable", "id=$round");
            if ($r == null)
                angeldie("failed accessing round with id: $round");
            $row = mysqli_fetch_row($r);
            if ($row == null)
                angeldie("round with id: $round does not exist");
            $round = $row[1];
        }
    }
    return($round);
}

function angelGetFriends($investorID) {
    $friends = array();

    $p = new angelPerson();
    $p->connect();

    $p->id = $investorID;
    $foundPerson = $p->findMatchingPerson(false);
    if ($foundPerson->friends != null) {
        $friendIDs = explode(",", $foundPerson->friends);
        foreach ($friendIDs as $id) {
            $p->id = intval($id);
            $f = $p->findMatchingPerson(false);
            $friends[] = $f;
        }
    }

    return($friends);
}

function angelGetAdvisors($investorID) {
    $advisors = array();

    $p = new angelPerson();
    $p->connect();

    $p->id = $investorID;
    $foundPerson = $p->findMatchingPerson(false);
    if ($foundPerson->advisors != null) {
        $advisorIDs = explode(",", $foundPerson->advisors);
        foreach ($advisorIDs as $id) {
            $p->id = intval($id);
            $f = $p->findMatchingPerson(false);
            $advisors[] = $f;
        }
    }

    return($advisors);
}
/*
 * return a truncated version of the input string (if long enough put a eli[se (...) at the end
 */
function angelTruncateString($s, $len) {
    $slen = strlen($s);
    if ($slen <= $len)      // no need to truncate, it's not long enough
        return($s);

    if ($len <= 3) {
        if ($slen > 3)
            return(substr($s, 0, $len));
        else
            return($s);
    } else {
        if ($slen > $len-3)
            return(substr($s,0,$len-3) . "...");
        else
            return(substr($s,0,$slen-3) . "...");
    }
}



function angelCreateSmartLink($handle, $type) {

    if ($type == "twitter") {
        /*
         * first extract twitter handle (at first assume that's what you've got)
         */
        $link = "<a href='https://twitter.com/$handle' class='twitter-follow-button' data-show-count='false'>
        Follow @$handle</a><script async src='//platform.twitter.com/widgets.js' charset='utf-8'></script>";

    } elseif ($type == "linkedin") {
        $link = "<a href='https://www.linkedin.com/in/$handle'>
                                <img src='https://static.licdn.com/scds/common/u/img/webpromo/btn_liprofile_blue_80x15.png'
                                width='80' height='15' border='0'></a>";

    } elseif ($type == "facebook") {
        $link = "FB: <a href='https://www.facebook.com/$handle'>$handle</div>";

    } elseif ($type == "github") {
        $link = "<a class='github-button' href='https://github.com/$handle'
                            aria-label='Follow @$handle on GitHub'>Follow @$handle</a>";

    } else {
        $link = "<unknown link type>";
    }


    return($link);
}
function angelParens($left, $right) {
    return("(" . $left . "," . "$right" . ")");
}



?>
