<?php include_once("zheader.php"); ?>

<h1>Name <img src="d_arr.png" /></h1>

<p class="pleft">xmlds - Tags used in a Spark XML Data Source</p>

<h1>Description <img src="d_arr.png" /></h1>

<p class="block">Since the <a href="spec_sdf.php">SDF format</a> is very
unstable -- multiple styles have appeared in the years and a header does not
exist to say what kind it is -- a new Data Source module has been invented to
read user definitions from an XML file. It is a layer to libxml which takes
care of all the necessary stuff.</p>

<p class="block">Prerequisite is that you know some HTML/XML/XHTML.</p>

<p class="block">Begin the XML file with the <tt>&lt;datasource&gt;</tt> tag.
(Needed to recognize a file.)</p>

<h1>User object <img src="d_arr.png" /></h1>

<p class="block">The &lt;user&gt; tag specifies a user:</p>

<p class="pleft2"><tt>&lt;user nname="Engelhardt" vname="Jan" xuid="12345"
sgroup="11" /&gt;</tt></p>

<ul>
  <li><b>nname</b> -- surname (prenom)</b></li>
  <li><b>vname</b> -- first name. This and surname is used to build up a login
    name in Spark UI.</li>
  <li><b>xuid</b> -- a unique identification number, to distinguish users with
    the same name. The XUID can be any string you seem fit, as long as it is
    unique for this user. The birth date (or an encrypted / hashed version
    thereof) is often used.</li>
  <li><b>sgroup</b> -- Class (school, colleges), or just a transparent
    sub-group. The subgroup is mostly ignored, all the users in the input data
    will belong to one system group. It is only used for password printing in
    user interfaces.</li>
</ul>

<p class="block">Comments are allowed as XML/HTML allows them. They are
filtered out when parsing with libxml. Unknown tags or fields will also be
ignored.</p>

<?php include_once("zfooter.php"); ?>
