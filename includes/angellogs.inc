<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/10/14
 * Time: 11:41 PM
 */

class angellogs {

    public $error = "";
    public $view;

    function __construct() {
        $this->view = new angellogs_view();
    }

    /*******************
     * This method is the standard reg page
     */
    function showlogsPage($day, $search) {

        $logf = new angelLogfile();

        if ($search != NULL)
            $r = $logf->searchLogs($search);
        elseif ($day == NULL)
            $r = $logf->recentLogs(25);
        else
            $r = $logf->oneDayLogs($day);

        if (!$r)
            $this->error = "logfile access failed: (day,search): (" . $day . "," . $search . ")";
        else {
            $stats = $logf->logStats();
            if (!$stats)
                $this->error = "logfile access (stats) failed.";
        }

        $this->view->error = $this->error;
        $body = $this->view->logsPageBody($day,$r,$stats);
        $this->view->outputHTML($body);
    }

}


/*
 * View for angellogs
 */

class angellogs_view extends angelview {


    function __construct() {
        parent::__construct();

        $this->htmlHeader = "<title>Angelcalc Log Viewer</title>";

    }

    function logsPageBody($day, $r,$stats) {
        /*<a href='javascript:window.location.href=window.location.href' class='logsButton'>Refresh</a>*/
        if ($day == NULL)
            $period = "Recent";
        else
            $period = $day;

        $body = "<span style='font-size: large'>Angelcalc Logs ($period)</span><br>
                <script> $(function() {
                                         $( '#datepicker' ).datepicker();  }); </script>
                <div style='width=60%; margin: 0 0 10px 110px;text-align: left; font-size: medium'><form action='logs.php'method='POST'>
                         <a href='logs.php'>Most Recent</a> |
                                   <a href='logs.php?day=today'>Today</a>       |
                                   <a href='logs.php?day=yesterday'>Yesterday</a>   |
                                   Pick A Day: <input size='10%' type=text id='datepicker' name='day' value='$date' onchange='this.form.submit()'>
                                   <button class='logsButton'>find</button> <input size='10%' type=text id='logSearch' name='logSearch' value=''>

                                   </form>


                </div>

                <div id='container' style='border:0px solid black; float:left'>
                    <div id='navbar' style='border: 2px solid blue; position: fixed; top:215px;right:25px;'>
                    <table class='tr2 tr3 tr4' style='border:0px solid black; font-size: 100%' width='100%'>
                        <tr><td style='color:blue;padding-right:5px' align='right'>In last<hr></td>      <td>day<hr></td><td>week<hr></td><td>month<hr></td></tr>";
                            foreach(array_keys($stats) as $key) {
                                $x = $stats[$key][1];   // 0th entry is the log string (logstats() function)
                                $y = $stats[$key][2];
                                $z = $stats[$key][3];
                                $body .= "<tr><td>$key</td><td>$x</td><td>$y</td><td>$z</td></tr>";
                            }
        $body .= "
                    </table>
                </div>

                <div class='angellogs' id='content' style='width: 80%'>


                    <table class='' style='width=75%; table-layout: fixed'>
                ";
        $body .= $this->getLogRows($r);
        $body .= "

                        </table>
                    </div>
                    </div>
                    </div>
                ";



        return $body;
    }
    function getLogRows($r) {

        if ($this->error != "")
            $rows = "<th><td>$this->error</td></th>";
        else {
            $rows = "<tr><th>Date</th><th>Login (IP)</th><th>Event</th><th>[Referer --> Script]</th></tr>";
            $data = "";
            while($l = mysqli_fetch_object($r,"angelLogfile")){
                $data .= "<tr><td>$l->date</td><td>$l->user ($l->ip)</td><td style='width:35%;color:blue'>$l->description</td><td>[$l->refer --> $l->script]</td></tr>";
            }
            $rows .= $data;
        }
        return ($rows);

    }

}

