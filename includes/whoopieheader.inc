<html><head>
    <title>Whoopie</title>
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
                echo "<td style='border: 0px solid black' align=left><a href='$path/whoopie''><img height='40px' width='125px' src='$path/$logo'></a></td>";
                ?></td>
            <td>
                <a title='Whoopie IS New!' onClick=window.open('static/whoopieInstructions.html','CO','width=700,height=700,status=0,location=no,toolbar=no,menubar=no,0'); href='javascript:void(0);'>
                    <strong>Welcome to Whoopie!</strong></a>
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
                if ($id != -1)
                    $photo = $_SESSION["photo"];
                else
                    $photo = "";
                echo "<li><a href='/userProfile?id=$id' title=''> Hey You  " . $_SESSION["loggedInUserFname"] . '!';
                if ($photo != "")
                    echo "<br /><img src=$photo  class='circleThumbnail' height=30 width=30 />";
                echo '</a></li>';
                echo "
                    <li><a href='$path/whoopie' >Home</a></li>
                    <li><a href='javascript:void(0);' NAME='HELP'  title=''
                        onClick=window.open('static/whoopieInstructions.html','Whoopie','width=1000,height=750,0,status=0,');>Help</a></li>
                    <li><a href='$path/logout' >Logout</a></li>
                ";
                ?>

            </ul>
        </nav>
        </div>

    </div>
</div>
</div>

<div id=all style="clear:both; margin:25px 25px 45px 25px;">
</div>