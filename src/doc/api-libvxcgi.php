<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <i>libvxcgi</i> library provides functions commonly used
in CGI environments.</p>

<h1>Function overview</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxcgi/libvxcgi.h&gt;<br />
<br />
<b>int</b> vxcgi_authenticate(<b>const char *</b>user, <b>const char *</b>password);<br />
<b>char *</b>vxcgi_read_data(<b>int</b> argc, <b>const char **</b>argv);<br />
<b>struct</b> HXmap <b>*</b>vxcgi_split(<b>char *</b>str);
</code></p>

<h2>vxcgi_authenticate</h2>

<p class="block">Authenticate the user using the given password. Returns an AEE
code, carrying the PAM error code as a negative return value. (E.g.
<code>-7</code> for <code>PAM_AUTH_ERR</code> and <code>-10</code> for
<code>PAM_USER_UNKNOWN</code>.)</p>

<h2>vxcgi_read_data</h2>

<p class="block">Returns the CGI POST data, or in case of a GET query, the
query string. If called from the command-line, that is, if
<code>getenv("REQUEST_METHOD")&nbsp;== NULL</code>, a copy of
<code>argv[1]</code> is returned, which is assumed to be in
<i>application/x-www-form-urlencoded</i> encoding. Note that the returned
string is allocated and therefore must be freed at a later time.</p>

<p class="block">If you do not intend to pass any <i>argc</i>/<i>argv</i>, call
<code>vxcgi_read_data</code> with <code>(0, NULL)</code>.</p>

<h2>vxcgi_split</h2>

<p class="block">Splits the string according to
<i>application/x-www-form-urlencoded</i> rules and returns the key-value pairs
in a <code>struct HXmap</code>. The string must be writable, as it will be
modified and freed. The caller should preferably duplicate it beforehand.</p>

<h1>Example</h1>

<p class="block">Here is how to quickly get a HXmap out of the query
string:</p>

<p class="code"><code><b>struct</b> HXmap<b>*</b>data <b>=</b>
vxcgi_split(vxcgi_read_data(argc, argv));</code></p>

<p class="block">and to get the "user" parameter if the query string:</p>

<p class="code"><code><b>char *</b>u <b>=</b> HXmap_get(data, "user");</code></p>

<?php include_once("Base-footer.php"); ?>
