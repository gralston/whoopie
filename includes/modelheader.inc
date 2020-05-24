<html><head>
    <title>Angelcalc</title>
    <meta http-equiv='Content-Type' content='text/html;charset=UTF-8'>
</head>
<body style="margin:0px;">

<div id="navall" class="angelcalcTopNav" style="border: 0px solid black;padding-bottom: 60px;">
    <div style="float:left; border: 0px solid red">
        <table border=0 cellpadding=10px cellspacing=10px height=60px style="border: 0px solid black;";>
        <tr style="border: 0px solid black"><td>
            <?php
            $logo = $GLOBALS["logo"];
            $id = $_SESSION["loggedInUserID"];
            $fname = $GLOBALS['loggedInUserFname'];
            echo "<td style='border: 0px solid black' align=left><a href='/model''><img height='30px' width='200px' src='/$logo'></a></td>";
            ?>
        </td><td>
                <a title='Whats New?' onClick=window.open('/static/updatehelp.html','CO','width=700,height=700,status=0,location=no,toolbar=no,menubar=no,0'); href='javascript:void(0);'>
                    <strong>What's New?</strong></a>


            </td>



        </tr>
        </table>
    </div>
    <div style="float:right;vertical-align: top; border: 0px solid blue">
        <nav id="topNav">
            <div class="angelcalcTopNav" style="border: 0px solid black;">
            <ul>
                <?PHP
                echo "<li><a href='/userProfile?id=$id' title=''> Hi " . $fname . '!</a></li>';
                echo "

                <li><a href='/model' title='Model'>Home</a></li>
                <li><a href='/model?compare' title='Compare'>Compare</a></li>
                <li><a href='/model?help=start' title='Help'>Help</a></li>";

                if ($fname != "Guest") {
                    echo "<li><a href='/logout' title='Logout'>Logout</a></li>";
                } else {
                    echo "<li><a href='/login' title='Login'>Login</a></li>";
                    echo "<li><a href='/reg' title='Reg'>Sign Up</a></li>";
                }


            ?>
            </ul>
        </nav>
        </div>

    </div>
</div>
</div>

<div id=all style="margin:25px;">