/**
 * Created by geoff on 6/20/16.
 */

$(document).ready(function () {

    $.get("/investments?loadCompanies", function(companyData,status) {
        var cos = JSON.parse(companyData);
        Companies.jsonToList(cos);

    //Prepare jTable
    $('#tableViewerContent').jtable({
        title: 'Angelcalc Investments',
        paging: true,
        sorting: true,
        defaultSorting: 'id ASC',
        pageSize: 15,
        openChildAsAccordion: true,
        actions: {
                    listAction:   '/tables/angelinvestsActions.php?action=list',
                    createAction: '/tables/angelinvestsActions.php?action=create',
                    updateAction: '/tables/angelinvestsActions.php?action=update',
                    deleteAction: '/tables/angelinvestsActions.php?action=delete'


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
        company_id: {
            title: 'Co',
            display: function (data) {
                link = "<a href=/companyProfile.php?id=" + data.record.company_id + ">" + Companies.map[data.record.company_id].name + "</a>";
                return link;
                //return 'test';
            },
            width: '3%',
            columnResizeable : true

        },
        investor_id: {
            title: 'Inv',
            width: '3%',
            columnResizeable : true

        },
        inv_date: {
            title: 'Inv Date',
            width: '5%',
            columnResizeable : true

        },
        exit_date: {
            title: 'Exit Date',
            width: '5%',
            visibility: 'hidden',
            columnResizeable : true

        },
        round: {
            title: 'Round',
            width: '7%',
            columnResizeable : true

        },
        type: {
            title: 'Type',
            width: '7%',
            columnResizeable : true
        },

        invest_amount: {
            title: 'Amount',
            width: '10%',
            columnResizeable : true
        },
        prevaluation: {
            title: 'premoney val',
            columnResizeable : true,
            visibility: 'hidden',
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
        status: {
            title: 'Status',
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

}); // get