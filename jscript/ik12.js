/**
 * Created by geoff on 5/21/14.
 */
/*$('.ik12confirmation').on('click', function () {
    return confirm('Are you sure?');
    });
    */

/*
 * the following was added for ik12logs.inc, but did not work.  I need to learn why.

$(function() {
    $( "#datepicker" ).datepicker();
});
*/

function ik12ShowInvestors(str)
{
    if (str.length < 2)
    {
    document.getElementById("txtHint").innerHTML="";
    return;
    }

xmlhttp=new XMLHttpRequest();
xmlhttp.onreadystatechange=function()
  {
      if (xmlhttp.readyState==4 && xmlhttp.status==200)
      {
      document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
      }
}
xmlhttp.open("GET","../utility/livesearch.php?q="+str,true);
xmlhttp.send();
}

