<html><head>
    <title>Angelcalc</title>
    <meta http-equiv='Content-Type' content='text/html;charset=UTF-8'>
</head>
<body style="margin:0px;">
<div class="angelcalcTop">
<div id="navall" class="angelcalcTopNav" style="border: 0px solid black;padding-bottom: 60px;">
    <div style="float:left; border: 0px solid red">
        <table border=0 cellpadding=10px cellspacing=10px height=60px class="angelcalcTopNav">
        <tr style="border: 0px solid black"><td>
                <?php
                $logo = $GLOBALS["logo"];
                $path = $GLOBALS["path"];
                echo "<td style='border: 0px solid black' align=left><a href='$path/investments?display=investments''><img height='30px' width='200px' src='$path/$logo'></a></td>";
                ?></td>
            <td>
                <a title='Whats New?' onClick=window.open('/static/updatehelp.html','CO','width=700,height=700,0,status=0,location=no,toolbar=no,menubar=no'); href='javascript:void(0);'>
                    <strong>What's New?</strong></a>
            </td>



        </tr>
        </table>
    </div>
    <div style="float:right;z-index: 10; vertical-align: top; border: 0px solid blue">
        <nav id="topNav">
            <div class="angelcalcTopNav">
            <ul>
                <?PHP
                $id = $_SESSION["loggedInUserID"];
                $photo = $_SESSION["photo"];
                echo "<li><a href='/userProfile?id=$id' title=''> Hi " . $_SESSION["loggedInUserFname"] . '!';
                if ($photo != "")
                    echo "<br /><img src=$photo class='circleThumbnail' height=30 width=30 />";
                echo '</a></li>';
                echo "
                    <li><a href='$path/investments' >Home</a></li>
                    <li>
                        <a href='#'>Tables</a>
                        <ul>
                            <li><a href='$path/tables/companyTable' >Companies</a></li>
                            <li><a href='$path/tables/angelinvestsTable' >Investments</a></li>
                            <li><a href='$path/tables/personTable' >People</a></li>
                            <li><a href='$path/tables/modelTable' >Models</a></li>
                            <li><a href='$path/tables/convertTable' >Converts</a></li>
                             <li><a href='$path/tables/payoutTable' >Payouts</a></li>
                        </ul></li>
                    <li>
                        <a href='#'>Tools</a>
                        <ul>
                            <li><a href='$path/logs' >Logs</a></li>
                            <li><a href='$path/angelstats' >Stats</a></li>
                        </ul></li>
                    <li><a href='javascript:void(0);' NAME='HELP'  title=''
                        onClick=window.open('/investments?help=start','Angelcalc','width=1000,height=750,0,status=0,');>Help</a></li>

                    <li><a href='$path/logout' >Logout</a></li>
                ";
                ?>

            </ul>
        </nav>
        </div>

    </div>
</div>
</div>

<div id=all style="clear:both; margin:25px 25px 75px 25px;">
</div>