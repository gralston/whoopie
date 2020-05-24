<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/3/14
 * Time: 10:43 PM
 */

class angelview {
    public $dtd                 = ""; // DOC type tag
    public $htmlHeader          = ""; //used for <meta> tags
    public $title               = ""; // used for the title for the html page - need to include the <title></title> tags
    public $css                 = ""; // used to link to css files, need to use the <link> tag
    public $js                  = ""; // used to link to js files, need to use the <script> tag
    public $header              = ""; // the html output for the page header (ie - ik12 logo and navigation)
                                      /* current choices include:
                                            header_basic.inc - used for applicants
                                            header_founder.inc - used for ik12founders in
                                            header_nav.inc - for internal use
                                            header_no_searchbox.inc - for internal use
                                            header_w_search.inc - for internal use
                                      */
    public $customHeader        = ""; // create your own html output header
    public $footer              = "";



    function __construct() {


        $path = $GLOBALS["path"];

        if (isset($_SESSION['auth']))
            $auth = $_SESSION['auth'];
        else
            $auth = "founder";  // lowest level

        if ($auth == "root")
            $this->header       = "angelAdminHeader.inc";
        else
            $this->header       = "angelheader.inc";

        if ($auth == "root")
            $angelJS = "$path/jscript/angel.js?v=" . time();       // this will ensure we reload css every time for debug
        else
            $angelJS = "$path/jscript/angel.js";

        $this->js .= "
                    <script src='$path/jscript/jquery-1.11.1.js'></script>
                    <script src='$path/jscript/jquery-1.11.1.min.js'></script>
                    <script src='$path/jscript/select2/select2.js'></script>
                    <script src='$path/jscript/jquery-ui-1.11.0/jquery-ui.js'></script>
                    <script src='$path/jscript/jquery.number2.5.js'></script>
                    <script src='$path/jscript/js.cookie.js'></script>
                    <script src='$path/jscript/clipboard.min.js'></script>
                    <script src='$path/jscript/notify.min.js'></script>
                    <script src='$path/jscript/jquery.feedback_me.js'></script>
                    <script async defer src='https://buttons.github.io/buttons.js'></script>

                    <script src='$angelJS'></script>
                    <script src='https://www.gstatic.com/charts/loader.js'></script>
                    ";

        $this->js .= '
            <script>
                $(function() {
                    $( "#datepicker" ).datepicker();
                 });
            </script>';
        $this->js .= '<script>$(document).ready(function(){$("#note").click(function(){$(".add").toggle("fast");});});</script>';

        $this->css          = "<link href='$path/css/ik12.css' type='text/css' rel='stylesheet' />";        // LOSE THIS!
        if ($auth == "root")
            $angelCSS = "$path/css/angel.css?v=" . time();       // this will ensure we reload css every time for debug
        else
            $angelCSS = "$path/css/angel.css";

        $this->css .= "

            <link href='$path/jscript/select2/select2.css' rel='stylesheet'>
            <link href='$path/jscript/jquery-ui-1.11.0/jquery-ui.css' rel='stylesheet'>
            <link href='$path/css/jquery.feedback_me.css' rel='stylesheet'>

            <link href='$angelCSS' rel='stylesheet'>

            ";
        // <link href='$path/css/bootstrap.css' rel='stylesheet'>                       <script src='$path/jscript/bootstrap.js'></script>

        $this->footer       = "angelfooter.inc";

        $this->dtd          = '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"><html>';
        $this->title        = "";


    }  // construct()


    function outputHTML($body) {
        echo $this->dtd;
        echo "<head>";
		echo $this->title;
        echo $this->css;
        echo $this->js;
             //  <html xmlns="http://www.w3.org/1999/xhtml">';
        echo $this->htmlHeader;
        echo "</head>";
        echo "<body>";
        if ($this->customHeader != "")
            echo $this->customHeader;
        else
            require($this->header);

        echo "<div class='angelwrapper'>";
        echo $body;
        echo "</div></div></div>";
        echo '<div class="angelpush"></div></div>';
        echo '</div></div></div><br>';
        require($this->footer);
        echo "</body>";
        echo "</html>";
    }

} 