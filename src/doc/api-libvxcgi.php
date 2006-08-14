<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <tt>libvxcgi</tt> library provides functions commonly used
in CGI environments.</p>

<h1>Function overview</h1>

<p class="code"><tt>
<b>#</b>include &lt;libvxcgi/libvxcgi.h&gt;<br />
<br />
<b>int</b> vxcgi_authenticate(<b>const char *</b>user, <b>const char *</b>password);<br />
<b>char *</b>vxcgi_read_data(<b>int</b> argc, <b>const char **</b>argv);<br />
<b>struct</b> HXbtree <b>*</b>vxcgi_split(<b>char *</b>str);
</tt></p>

<h2>vxcgi_authenticate</h2>

<p class="block">Authenticate the user. Returns an AEE code, carrying the PAM
error code as a negative return value. (E.g. <tt>-7</tt> for
<tt>PAM_AUTH_ERR</tt> and <tt>10</tt> for <tt>PAM_USER_UNKNOWN</tt>.)</p>

<h2>vxcgi_read_data</h2>

<p class="block">Returns the CGI POST data, or in case of a GET query, the
query string. If called from the command-line, that is, if
<tt>getenv("REQUEST_METHOD") == NULL</tt>, a copy of <tt>argv[1]</tt> is
returned, which is assumed to be in <i>application/x-www-form-urlencoded</i>
encoding. Note that the returned string is allocated and therefore must be
freed at a later time.</p>

<p class="block">If you do not intend to use <tt>argc</tt>/<tt>argv</tt>, call
<tt>vxcgi_read_data(0, NULL)</tt>.</p>

<h2>vxcgi_split</h2>

<p class="block">Splits the string according to
<i>application/x-www-form-urlencoded</i> rules and returns the key-value pairs
in a <tt>struct HXbtree</tt>. The string must be writable, as it will be
modified and freed. The caller should preferably duplicate it beforehand.</p>

<h1>Example</h1>

<p class="block">Here is how to quickly get a HXbtree out of the query
string:</p>

<p class="code"><tt><b>struct</b> HXbtree <b>*</b>data <b>=</b>
vxcgi_split(vxcgi_read_data(argc, argv));</tt></p>

<p class="block">and to get the "user" parameter if the query string:</p>

<p class="code"><tt><b>char *</b>u <b>=</b> HXbtree_get(data, "user");</tt></p>

<?php include_once("Base-footer.php"); ?>
