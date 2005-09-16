<?php include_once("zheader.php"); ?>

<h1>Name <img src="d_arr.png" /></h1>

<p class="pleft">sdf - Specifications of the SDF file format</p>

<h1>Description <img src="d_arr.png" /></h1>

<p class="block">An SDF file is a text file, where each object occupies one
line. Empty lines are ignored. The structure of a User Object is similar to a
CSV file; in SDF, the values are separated by a semicolon. Below is the
schematic description is a real-world example.</p>

<p class="pleft2"><tt>[enum;]nname;vname;xuid;sgroup<br />
(* one-line comment *)<br />
# new-style comment</tt></p>

<p class="pleft2"><tt>1;Engelhardt;Jan;12.3.45;Grade 12</tt></p>

<p class="block">The different fields are:</p>

<ul>
  <li><b>enumeration number</b> -- this value is generally ignored since
    unused. It is usually an enumeration number within the class /
    subgroup. Some SDF export programs do not provide this field at all.</li>
  <li><b>nname</b> -- surname (prenom)</b></li>
  <li><b>vname</b> -- first name. This and surname is used to build up a login
    name in Spark UI.</li>
  <li><b>xuid</b> -- a unique identification number, to distinguish users with
    the same name. The XUID can be any string you seem fit, as long as it is
    unique for this user. The birth date is often used.</li>
  <li><b>sgroup</b> -- Class (school, colleges), or just a transparent
    sub-group. The subgroup is mostly ignored, all the users in the input data
    will belong to one system group. It is only used for password printing in
    user interfaces.</li>
</ul>

<h1>Comments <img src="d_arr.png" /></h1>

<p class="block">Comments are Pascal-style or similar to shell and C, as shown
in the syntax above, they either begin with a hash mark in the first column, or
start with <tt>(*</tt> <i>and</i> end with <tt>*)</tt>. Both comment types only
last for the particular line. The hash-mark style comment may be unsupported
within Kolleg. Comments can only begin in the first column, i.e. the following
is illegal (or at least, is not supported by <tt>sdf2xml</tt>):</p>

<p class="pleft2"><tt>Surname;Name;Birthdate;Class (* comment *)<br />
Surname;Name;Birthdate;Class # comment</tt></p>

<h1>Portability issues <img src="d_arr.png" /></h1>

<p class="block">The only application I know that exports SDF files is "Kolleg"
(a secretary program for German colleges). It is a DOS/16 program and thus
exports in ASCII/CRLF mode. Anyone wishing to use Spark should use the
<tt>sdf2xml</tt> utility. Under circumstances, you probably will need to modify
it to suit your SDF specs.</p>

<p class="block">StarOffice also seems to export something with the
<tt>.sdf</tt> extension, so do not be confused!</p>

<p class="block">If the newline termination sequence is only <tt>\x0A</tt>, the
ANSI set is assumed! See <tt>sdf2xml</tt>'s <tt>-a</tt> option to force ANSI
mode even if CRLF is detected.</p>

<h1>Bugs <img src="d_arr.png" /></h1>

<p class="block">The newline termination sequence scanner can not truly run
automated, as having CRLF is NO indication of using ASCII. The <tt>-a</tt>
option may help, though.</p>

<?php include_once("zfooter.php"); ?>
