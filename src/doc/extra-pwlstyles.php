<?php include_once("Base-header.php"); ?>

<h1>a1</h1>

<p class="block">Prints exactly one table containing all users in
text/plain.</p>

<h1>pg_html</h1>

<p class="block">Generates one table per private group descriptor and outputs
in text/html.</p>

<h1>pg_rtf</h1>

<p class="block">Generates one table per private group descriptor, with a page
break after each table. The output format is text/rtf, which can be read by
e.g. Microsoft Office. This style requires a template file; there are two
templates shipped, <code>share/sg.rtf</code> (English) and
<code>share/sg_DE.rtf</code> (German).</p>

<h1>pg_txt</h1>

<p class="block">Generates one table per private group descriptor, with a page
break after each table (by ASCII fashion this is the form-feed character,
<code>'\f'</code>). The output format is text/plain, and line endings are what
the system outputs for <code>'\n'</code>. Under UNIX this means it is just
<code>'\n'</code> and you will need an extra preprocessing step to convert this
to <code>'\r\n'</code> if sending it directly to the printer over
<code>/dev/lp*</code>.</p>

<h1>sb</h1>

<p class="block">Generates a nice form for each user where his/her login
details and possibly other info is noted. The output is a TeX document.</p>

<p class="block">The purpose of this style is that each user gets a
letter, possibly wrapped into an envelope to not unnecessarily reveal
the password or other sensitive information.</p>

<?php include_once("Base-footer.php"); ?>
