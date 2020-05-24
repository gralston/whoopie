/**
 * Created by geoff on 6/20/16.
 */

$(document).ready(function () {

    //Prepare jTable
    $('#tableViewerContent').jtable({
        title: 'Angelcalc Models',
        paging: true,
        sorting: true,
        defaultSorting: 'creationDate DESC',
        pageSize: 15,
        openChildAsAccordion: true,
        actions: {
                    listAction:   '/tables/modelActions.php?action=list',
                    createAction: '/tables/modelActions.php?action=create',
                    updateAction: '/tables/modelActions.php?action=update',
                    deleteAction: '/tables/modelActions.php?action=delete'


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

    investor_id: {
        title: 'Inv ID',
        width: '10%',
        columnResizeable : true,
        visibility: 'fixed',
        display: function (data) {
        link = "<a href=/profile.php?id=" + data.record.investor_id + ">" + data.record.investor_id + "</a>";
        return link;
        //return 'test';
        }
    },
    creationDate: {
        title: 'Created',
        width: '10%',
        columnResizeable : true
        },
    name: {
        title: 'Name',
        width: '10%',
        columnResizeable : true
        },
    numberConverts: {
        title: 'Converts',
        width: '10%',
        columnResizeable : true
        },
    vcInvestment: {
        title: 'VC Inv',
        width: '10%',
        columnResizeable : true
        },
    yourInvestment: {
        title: 'Your Inv',
        width: '10%',
        columnResizeable : true
    },
    preValuation: {
        title: 'Pre Val',
        columnResizeable : true,
        width: '10%'
        },
    postOptionsPercent: {
        title: 'Post Options',
        columnResizeable : true,
        width: '10%'
    },
    fdPreMoneyShares: {
        title: 'FD Shares Pre',
        width: '10%'
    },
    preMoneyUnallocOptions: {
            title: 'Pre Options',
            columnResizeable : true,
            width: '10%'
    },
    refcount: {
        title: 'ref',
        columnResizeable : true,
        width: '10%'
    },
    sharing: {
        title: 'Sharing',
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
                name: $('#name').val()
            });
        });

        //Load all records when page is first shown
        $('#LoadRecordsButton').click();


    });

