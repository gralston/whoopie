/**
 * Created by geoff on 6/20/16.
 */

$(document).ready(function () {

    //Prepare jTable
    $('#tableViewerContent').jtable({
        title: 'Angelcalc Payouts',
        paging: true,
        sorting: true,
        defaultSorting: 'id ASC',
        pageSize: 15,
        openChildAsAccordion: true,
        actions: {
                    listAction:   '/tables/payoutActions.php?action=list',
                    createAction: '/tables/payoutActions.php?action=create',
                    updateAction: '/tables/payoutActions.php?action=update',
                    deleteAction: '/tables/payoutActions.php?action=delete'


       },
        toolbar: {
            hoverAnimation: true, //Enable/disable small animation on mouse hover to a toolbar item.
            hoverAnimationDuration: 60, //Duration of the hover animation.
            hoverAnimationEasing: undefined, //Easing of the hover animation. Uses jQuery's default animation ('swing') if set to undefined.
            items: []
        },
    fields: {          /* note that these must be identical to the values returned by mysql! */
    id: {
        title: 'ID',
        width: '3%',
        key: true,
        create: false,
        edit: false,
        list: true
        },
        companyID: {
            title: 'Co',
            width: '3%',
            columnResizeable : true

        },
        investorID: {
            title: 'Inv',
            width: '3%',
            columnResizeable : true

        },
        payDate: {
            title: 'Pay Date',
            width: '5%',
            columnResizeable : true

        },
        cash: {
            title: 'Cash',
            width: '5%',
            columnResizeable : true

        },
        shares: {
            title: 'Shares',
            width: '5%',
            columnResizeable : true
        },
        symbol: {
            title: 'Symbol',
            width: '5%',
            columnResizeable : true
        },

        sharePrice: {
            title: 'Price',
            width: '5%',
            columnResizeable : true
        },
    last_update: {
        title: 'Updated',
        // width: '200px',
        type: 'date',
        visibility: 'hidden',
        create: false,
        edit: false
        }

    }
    });
    /*
    * Must add filters here as well.  Then re-load records when user click 'load records' button.
    */


    $('#LoadRecordsButton').click(function (e) {
            e.preventDefault();
            $('#tableViewerContent').jtable('load', {
                model_id: $('#model_id').val()
            });
        });

        //Load all records when page is first shown
        $('#LoadRecordsButton').click();


    });

