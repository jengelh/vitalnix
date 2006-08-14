<?php include_once("Base-header.php"); ?>

<h1>Abstract</h1>

<p>Tags used in a XML Data Source</p>

<h1>Description</h1>

<p class="block">The <a href="spec-sdf.php">SDF</a> format has a number of
issues, such as having no clear identification and layout description. An
alternative format based upon XML has therefore been described and can be input
to <i>libvxeds</i>.

<p class="block">The following keys are recognized:</p>

<ul>
  <li><b>surname</b>&nbsp;-- last name</li>
  <li><b>firstname</b>&nbsp;-- first name<br />
    This field is mandatory.</li>
  <li><b>pvgrp</b>&nbsp;-- private group descriptor<br />
    This can be anything you want, and is commonly used for school classes. It
    will only ever be used for password printing in user interfaces, or for
    custom applications that use it.</li>
  <li><b>uuid</b>&nbsp;-- an arbitrary string that uniquely identifies this
    user within the Data Source<br />
    It is preferred that it consists only of characters in
    [<tt>0-9A-Za-z_</tt>], because this uuid might be copied to a passwd(5)
    style file.</li>
  <li><b>bday</b>&nbsp;-- birth date<br />
    if no <tt>uuid</tt> is given, create one based on name and this date
    (commonly birth date). Formats accepted are DD.MM.YYYY, MM/DD/YYYY and
    YYYY-MM-DD.</li>
</ul>

<p class="block">Comments are allowed as far as XML allows them. They are
filtered out when parsing with libxml. Unknown tags or fields will also be
ignored.</p>

<h1>Sample</h1>

<p class="code"><tt>
&lt;?xml version="1.0" encoding="UTF-8" ?&gt;<br />
&lt;VX3_xmlds&gt;<br />
&nbsp; &nbsp; &lt;user surname="Engelhardt" firstname="Jan" pvgrp="13" uuid="12345678" /&gt;<br />
&lt;/VX3_xmlds&gt;
</tt></p>

<?php include_once("Base-footer.php"); ?>
