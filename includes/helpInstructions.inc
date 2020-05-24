


    Angelcalc models the conversion of safes and/or convertible notes during an equity financing. It uses
    techniques similar to those used by Microsoft Excel to make its calculations. The instructions below outline the
    steps necessary to create and save a model of a particular financing. A sample model with two safes and a
    $2,000,000 financing with a $8,000,000 pre-money valuation is automatically included to help demonstrate
    what a conversion looks like. Detailed instructions for how to create your own model follow below.


    <P></P>

    STEP 1: For each convertible in the deal, click the <span style="text-decoration: underline">Add New</span> link to add that convertible to the model.
    You will need the total investment in that convertible, including any interest, and the discount (if any) and cap (if any).  You may also include
        any investment you made using this convertible.

    <p></p>
    STEP 2: Fill in the following fields for the financing:

    <div style="margin: 10px 0px 0px 30px">
        <table style="color:black; padding: 20px 20px 20px 20px;border-spacing: 15px 0px;">
            <tr><td width="150px" style="color:blue">Your Investment</td><td>If you are an angel and invested in the round itself, enter the amount here.</td></tr>
            <tr><td colspan="2"><hr></td></tr>
            <tr><td  style="color:blue">Total Investment</td><td>This is the total amount invested directly in the equity round (including yours, of course).</td></tr>
            <tr><td colspan="2"><hr></td></tr>
            <tr><td style="color:blue">Options / Post</td><td>This is the post-money unallocated pool of options as a percent of the total fully
                    diluted post-money shares. This percentage is usually negotiated.</td></tr>
            <tr><td colspan="2"><hr></td></tr>
            <tr><td style="color:blue">Valuation</td><td>This is the company&#8217s negotiated value, either pre or post money. </td></tr>
            <tr><td colspan="2"><hr></td></tr>
            <tr><td style="color:blue">Fully diluted pre-money shares</td><td>This is the company&#8217s total fully diluted shares outstanding prior to this deal,
                    including all allocated and unallocated options.</td></tr>
            <tr><td colspan="2"><hr></td></tr>
            <tr><td style="color:blue">Unallocated pre-money options</td><td>This is the count of options from any pre-money pool
                                      which remain to be allocated. They ARE included in the fully diluted
                                      pre-money share count above.
                </td></tr>

        </table>
    </div>

    STEP 3: Click one of the modeling buttons to create your model. You may model:
    <div style="margin: 10px 0px 20px 50px; color:black">
    <ol style="list-style-position: outside; margin-left: 10px">
            <li>The initial cap table before any money is taken via convertibles</li>
            <li>The notional cap table that would exist if all convertibles converted prior to any equity round</li>
            <li>The cap table which will exist after the equity round is complete</li>
     </ol></div>
<p></p>
    You may specify the number of decimal places of precision that the
    model uses as well. This is helpful to match Excel models where the modeler chooses to round their calculations.
    The model may also be named and saved (by clicking the "Save" button),
    but this does require a (simple) registration.
    <p></p>
    You may compare two different models or variations on the same model by clicking <a href="/model?compare"> Compare</a> (also in the nav bar).

  <p></p> <hr><p></p>

    <span style="color: red">Note: Convertibles.</span><br />
    The following convertibles are supported:<p></p>
    <div style="margin: 10px 0px 20px 50px; color:black">
        <ol style="list-style-position: outside; margin-left: 10px">
            <li><span style="color:blue">Standard Y Combinator (YC) post-money safe</span> (simple agreement for future equity) with a post-money cap and / or discount.
            New options defined in the equity round are NOT included in the pre-money for this convertible, although existing, but
                unissued one are.  In fact, when you choose the post money safe you can choose the YC standard or customize
                as you like in terms of whether options are included or not.

            </li>
            <li><span style="color:blue">Standard Y Combinator (YC) pre-money safe</span> (simple agreement for future equity) with a pre-money cap and / or discount.
            Note that all options, including new ones defined in the converting equity round
                as well as any existing options are assumed to be in the pre-money and thus do not dilute this convertible.
            </li>
            <li><span style="color:blue">Standard YC convertible note</span>. This is a debt instrument and will usually include an
                interest rate and a maturity date - neither of which are modeled by Angelcalc.
            </li>


            <li><span style="color:blue">Custom Convert</span>: This convertible is customizable in terms of when the cap or discount is used as
                well as the key components of the denominator used to divide the cap / valuation.</li>
            <li><span style="color:blue">Manual Convert</span>: This convertible can be used in the event a convertible type is not known or supported.
                In that case you may simply enter the investment amount and the number of shares into which that
                investment converted. These will then be incorporated into the model.

            <li><span style="color:blue">YCVC Safe</span>: specialized vehicle for YC companies through Summer 2017.</li>
            <li><span style="color:blue">YCII Safe</span>: specialized vehicle for YC companies Summer 2018 and Winter 2019</li>
            <li><span style="color:blue">YCII/III Post Safe</span>: specialized vehicle for YC companies Winter 2019 and beyond</li>

            </li>
        </ol>

    </div>
    Please note that interest is not calculated by the model. Rather, it is assumed that these
    amounts (your interest and total interest for all investors) will be provided by the company and entered as part of your
    investment and the total investment for any convertible which includes an interest rate (e.g. a convertible note).

    <p></p> <hr><p></p>
    <span style="color: red">Note: Calculations</span><br />
    If there is any doubt as to the calculation used to determine the price at which a convertible converts and
    therefore the number of shares into which it converts, the "Show Calc" link next to each entry in the cap table
    gives the details of each calculation.

    <p></p> <hr><p></p>
    <span style="color: red">Note: Accuracy</span><br />
    It is highly unlikely that this model will give precisely the same values as an Excel model due to rounding differences
    even if the precision chosen is the same as used in Excel.
    </span>


    <p></p> <hr><p></p>

    <span style="color: red">Disclaimer</span><br />
    The values calculated by this model are for comparison purposes only. Their accuracy is not guaranteed by the
    author. It is likely that this model will differ from Excel models of the same capitalization table, even when
    the inputs are identical due to rounding errors. No representation is made in any way as to which is the "correct"
    model in this case.
    </span>

