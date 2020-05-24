/**
 * Created by geoff on 6/20/16.
 */

$(document).ready(function () {

    //Prepare jTable
    $('#tableViewerContent').jtable({
        title: 'Angelcalc Converts',
        paging: true,
        sorting: true,
        defaultSorting: 'id ASC',
        pageSize: 15,
        openChildAsAccordion: true,
        actions: {
                    listAction:   '/tables/convertActions.php?action=list',
                    createAction: '/tables/convertActions.php?action=create',
                    updateAction: '/tables/convertActions.php?action=update',
                    deleteAction: '/tables/convertActions.php?action=delete'


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
        key: true,
        create: false,
        edit: false,
        list: true
        },

    model_id: {
        title: 'Model ID',
        width: '10%',
        columnResizeable : true

        },
    type: {
        title: 'Type',
        width: '10%',
        columnResizeable : true
        },

    yourInvestment: {
        title: 'Your Inv',
        width: '10%',
        columnResizeable : true
    },
    totalInvestment: {
        title: 'Total Inv',
        columnResizeable : true,
        width: '10%'
        },
    cap: {
        title: 'Cap',
        columnResizeable : true,
        width: '10%'
        },
    discount: {
        title: 'Discount',
        columnResizeable : true,
        width: '10%'
    },
    custom: {
            title: 'Custom',
            columnResizeable : true,
            width: '10%'
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

