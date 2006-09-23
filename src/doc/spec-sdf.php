<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">An SDF file is a text file, where each object occupies one
line. Empty lines are ignored. The structure of a User Object is similar to a
CSV file; in SDF, the values are separated by a semicolon. Below is the
schematic description is a real-world example.</p>

<p class="code">[<tt>enum;</tt>]<tt>surname;firstname;bday;pvgrp<br />
(* one-line comment *)<br />
# new-style comment</tt></p>

<p class="code"><tt>1;Engelhardt;Jan;12.3.45;Grade 13;</tt></p>

<p class="block">The different fields are:</p>

<ul>
  <li><b>enumeration number</b><br />
    this value is generally unused. It is usually an enumeration number within
    the pvgrp. Some SDF export programs do not provide this field at all.</li>
  <li><b>surname</b></li>
  <li><b>firstname</b></li>
  <li><b>bday</b>&nbsp;-- birth date</li>
    A UUID is built from the name and this date to uniquely identify the user
    within the Data Source. Formats accepted are DD.MM.YY, DD.MM.YYYY,
    MM/DD/YYYY and YYYY-MM-DD.</li>
  <li><b>pvgrp</b>&nbsp;-- private group descriptor<br />
    This can be anything you want, and is commonly used for school classes. It
    will only ever be used for password printing in user interfaces, or for
    custom applications that use it.</li>
</ul>

<h1>Comments</h1>

<p class="block">Comments are Pascal-style or similar to shell and C, as shown
in the syntax above, they either begin with a hash mark in the first column, or
start with <tt>(*</tt> <i>and</i> end with <tt>*)</tt>. Both comment types only
last for the particular line. The hash-mark style comment may be unsupported
within Kolleg. Comments can only begin in the first column.</p>

<h1>Portability issues</h1>

<p class="block">The only application I know that exports SDF files is "Kolleg"
(a secretary program for German schools). It is a DOS/16 program and thus
exports in CP437 encoding and CRLF mode. Since it is just like CSV format,
there is no indication of what fields describe what. If the SDF created by your
Kolleg program varies from the spec above, you need to change the parser to
properly recognize it, or contact the Vitalnix Project.</p>

<p class="block">StarOffice also seems to export something with the
<tt>.sdf</tt> extension, but it is not related!</p>

<h1>Naming</h1>

<p class="block">I think SDF stands for "Schülerdatenformat".</p>

<?php include_once("Base-footer.php"); ?>
