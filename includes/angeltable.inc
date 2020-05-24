<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/10/14
 * Time: 11:41 PM
 */

class angeltable {

    public $error = '';
    public $view;

    function __construct() {
        $this->view = new angeltable_view();
    }

    /*******************
     * This method is the standard table viewer
     */
    function showModelTablePage() {

        $this->view->error = $this->error;
        $body = $this->view->modelTablePageBody();
        $this->view->outputHTML($body);
    }

    function showConvertTablePage() {

        $this->view->error = $this->error;
        $body = $this->view->convertTablePageBody();
        $this->view->outputHTML($body);
    }

    function showCompanyTablePage() {

        $this->view->error = $this->error;
        $body = $this->view->companyTablePageBody();
        $this->view->outputHTML($body);
    }

    function showPersonTablePage() {

        $this->view->error = $this->error;
        $body = $this->view->personTablePageBody();
        $this->view->outputHTML($body);
    }
    function showangelinvestsTablePage() {

        $this->view->error = $this->error;
        $body = $this->view->angelinvestsTablePageBody();
        $this->view->outputHTML($body);
    }
    function showpayoutTablePage() {

        $this->view->error = $this->error;
        $body = $this->view->payoutTablePageBody();
        $this->view->outputHTML($body);
    }
}


/*
 * View for angellogs
 */

class angeltable_view extends angelview {


    function __construct() {
        parent::__construct();

        $path = $GLOBALS["path"];

        $this->htmlHeader = '<title>Angelcalc Table Viewer</title>';

        $this->css .=   "<link href='/jscript/jtable/jtable.2.4.0/ui-lightness/jquery-ui-1.10.4.css' rel='stylesheet' type='text/css' />
                         <link href='/jscript/jtable/jtable.2.4.0/themes/lightcolor/blue/jtable.min.css' rel='stylesheet' type='text/css' />";


        $this->js .= "<script src='/jscript/jtable/jtable.2.4.0/jquery-ui-1.10.4.min.js' type='text/javascript'></script>
                      <script src='/jscript/jtable/jtable.2.4.0/jquery.jtable.js' type='text/javascript'></script>";

    }

    function modelTablePageBody() {

        $path = $GLOBALS["path"];

        $this->js .= "<script src='/jscript/angelTabModel.js'></script>";

        $body = "

            <div id='tableViewerContainer' style='border: 0px solid black'>

              <div id='tableViewerNavbar' class='filtering' style='border: 0px solid black'>

                <form method='post'>
                    <table>
                        <tr><td colspan='2'> <input type='reset' value='Reset Filter'> </td></tr>
                        <tr><td>Name: </td><td><input type='text' name='name' id='name'></td></tr>
                        <tr><td>Email: </td><td><input type='text' name='email' id='email'></td></tr>
                        <tr><td>ID: </td><td><input type='text' name='id' id='id'></td></tr>
                        <tr><td>Company (exact match):</td><td> <input type='text' name='companyName' id='companyName' /></td></tr>
                        </td></tr>
                         <tr><td colspan='2'><button type='submit' id='LoadRecordsButton' class='filtering'>Filter</button></td></tr>
                    </table>
                </form>
              </div>


              <div id='tableViewerContent' style='border: 0px solid black'>

              </div>
            </div>
        ";

        return $body;
    }

    function convertTablePageBody() {

        $path = $GLOBALS["path"];

        $this->js .= "<script src='/jscript/angelTabConvert.js'></script>";

        $body = "

            <div id='tableViewerContainer' style='border: 0px solid black'>

              <div id='tableViewerNavbar' class='filtering' style='border: 0px solid black'>

                <form method='post'>
                    <table>
                        <tr><td colspan='2'> <input type='reset' value='Reset Filter'> </td></tr>
                        <tr><td>Model ID: </td><td><input type='text' name='model_id' id='model_id'></td></tr>
                        </td></tr>
                         <tr><td colspan='2'><button type='submit' id='LoadRecordsButton' class='filtering'>Filter</button></td></tr>
                    </table>
                </form>
              </div>


              <div id='tableViewerContent' style='border: 0px solid black'>

              </div>
            </div>
        ";

        return $body;
    }

    function companyTablePageBody() {


        $this->js .= "<script src='/jscript/angelTabCompany.js'></script>";

        $body = "

            <div id='tableViewerContainer' style='border: 0px solid black'>

              <div id='tableViewerNavbar' class='filtering' style='border: 0px solid black'>

                <form method='post'>
                    <table>
                                   <tr><td colspan='2'> <input type='reset' value='Reset Filter'> </td></tr>
            <tr><td>Name:</td><td> <input type='text' name='name' id='name' /></td></tr>
            <tr><td>Kind:</td><td>
                    <select id='kind' name='kind'>
                        <option selected='selected' value=''>All kinds</option>
                        <option value='startup'>Startup</option>
                        <option value='corporation'>Corp</option>
                        <option value='vc'>VC</option>

                    </select></td></tr>
            <tr><td>Status:</td><td>
                    <select id='status' name='status'>
                        <option selected='selected' value=''>All statuses</option>
                        <option value='active'>Active</option>
                        <option value='dead'>Dead</option>
                        <option value='unknown'>Unknown</option>

                    </select></td></tr>
            <td colspan='2'><button type='submit' id='LoadRecordsButton' class='filtering'>Filter</button></td></tr>
                    </table>
                </form>
              </div>


              <div id='tableViewerContent' style='border: 0px solid black'>

              </div>
            </div>
        ";

        return $body;
    }

    function angelinvestsTablePageBody() {

        $path = $GLOBALS["path"];

        $this->js .= "<script src='$path/jscript/angelTabAngelinvests.js'></script>";

        $body = "

            <div id='tableViewerContainer' style='border: 0px solid black'>

              <div id='tableViewerNavbar' class='filtering' style='border: 0px solid black'>

                <form method='post'>
                    <table>
                        <tr><td colspan='2'> <input type='reset' value='Reset Filter'> </td></tr>
                        <tr><td>Model ID: </td><td><input type='text' name='model_id' id='model_id'></td></tr>
                        </td></tr>
                         <tr><td colspan='2'><button type='submit' id='LoadRecordsButton' class='filtering'>Filter</button></td></tr>
                    </table>
                </form>
              </div>


              <div id='tableViewerContent' style='border: 0px solid black'>

              </div>
            </div>
        ";

        return $body;
    }


    function payoutTablePageBody() {

        $path = $GLOBALS["path"];

        $this->js .= "<script src='$path/jscript/angelTabPayout.js'></script>";

        $body = "

            <div id='tableViewerContainer' style='border: 0px solid black'>

              <div id='tableViewerNavbar' class='filtering' style='border: 0px solid black'>

                <form method='post'>
                    <table>
                        <tr><td colspan='2'> <input type='reset' value='Reset Filter'> </td></tr>
                        <tr><td>Payout ID: </td><td><input type='text' name='payout_id' id='payout_id'></td></tr>
                        </td></tr>
                         <tr><td colspan='2'><button type='submit' id='LoadRecordsButton' class='filtering'>Filter</button></td></tr>
                    </table>
                </form>
              </div>


              <div id='tableViewerContent' style='border: 0px solid black'>

              </div>
            </div>
        ";

        return $body;
    }

    function personTablePageBody() {

        $path = $GLOBALS["path"];

        $this->js .= "<script src='$path/jscript/angelTabPerson.js'></script>";

        $body = "

            <div id='tableViewerContainer' style='border: 0px solid black'>

              <div id='tableViewerNavbar' class='filtering' style='border: 0px solid black'>

                   <form method='post'>
        <table>
            <tr><td colspan='2'> <input type='reset' value='Reset Filter'> </td></tr>
            <tr><td>Name: </td><td><input type='text' name='name' id='name'></td></tr>
            <tr><td>Email: </td><td><input type='text' name='email' id='email'></td></tr>
            <tr><td>ID: </td><td><input type='text' name='id' id='id'></td></tr>
            <tr><td>Company (exact match):</td><td> <input type='text' name='companyName' id='companyName' /></td></tr>
            <tr><td>Gender:</td><td>
                    <select id='gender' name='gender'>
                        <option selected='selected' value=''>All Genders</option>
                        <option value='m'>Male</option>
                        <option value='f'>Female</option>
                        <option value='o'>Other</option>
                    </select></td></tr>
            <tr><td>Kind:</td><td>
                    <select id='kind' name='kind'>
                        <option selected='selected' value=''>All kinds</option>
                        <option value='angel'>angel</option>
                        <option value='educator'>educator</option>
                        <option value='founder'>founder</option>
                        <option value='ik12founder'>ik12 founder</option>
                        <option value='vcperson'>VC</option>
                        <option value='ik12person'>ik12 person</option>
                    </select></td></tr>

            <tr><td colspan='2'><button type='submit' id='LoadRecordsButton' class='filtering'>Filter</button></td></tr>
        </table>
    </form>
              </div>


              <div id='tableViewerContent' style='border: 0px solid black'>

              </div>
            </div>
        ";

        return $body;
    }


}

