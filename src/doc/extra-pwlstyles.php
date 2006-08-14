<?php include_once("Base-header.php"); ?>

<h1>"a1"</h1>

<p class="block">Prints exactly one table containing all users in
text/plain.</p>

<h1>"pg_html"</h1>

<p class="block">Generates one table per private group descriptor and outputs
in text/html.</p>

<h1>"pg_rtf"</h1>

<p class="block">Generates one table per private group descriptor, with a page
break after each table. The output format is text/rtf, which can be read by
e.g. Microsoft Office. This style requires a template file; there are two
templates shipped, <tt>share/sg.rtf</tt> (English) and <tt>share/sg_DE.rtf</tt>
(German).</p>

<h1>"pg_txt"</h1>

<p class="block">Generates one table per private group descriptor, with a page
break after each table (by ASCII fashion this is the form-feed character,
<tt>'\f'</tt>). The output format is text/plain, and line endings are what the
system outputs for <tt>'\n'</tt>. Under UNIX this means it is just
<tt>'\n'</tt> and you will need an extra preprocessing step to convert this to
<tt>'\r\n'</tt> if sending it directly to the printer over
<tt>/dev/lp*</tt>.</p>

<h1>"sb"</h1>

<p class="block">Generates a nice form for each user where his/her login
details and possibly other info is noted. The output is a TeX document.</p>

<?php include_once("Base-footer.php"); ?>
