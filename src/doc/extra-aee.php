<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>AEE - Alternate Error Encoding</p>

<h1>Description</h1>

<p class="block">Functions from the Vitalnix API work a little different than
the usual libc library calls when it comes to error reporting. The following
document will describe the Alternate Error Encoding, which other manpages refer
to.</p>

<p class="block">Different classes of functions have different possible return
values.</p>

<ol type="A">
  <li>functions returning success or error (a "boolean" value)</li>
  <li>functions returning success or an error code</li>
  <li>functions returning an integral value or an error code</li>
  <li>functions returning a pointer or an error code</li>
</ol>

<h1>Class A&nbsp;-- returning boolean</h1>

<p class="block">This class of functions returns non-zero for success and zero
for failure. An <code>if()</code> construct can therefore be short:</p>

<p class="code"><code>
if(<b>!</b>bool_function())<br />
&nbsp; &nbsp; printf("Total failure\n");
</code></p>

<p class="block">This AEE draft does not specify if and how <code>errno</code>
is set in case of a false&nbsp;-- this is specific to a function.</p>

<h1>Class B&nbsp;-- success or error code</h1>

<h2>negative, non-zero</h2>

<p class="block">In case of an error, the <code>errno</code>-style error code
is returned as a negative number, e.g. <code>-ENOMEM</code>. This differs from
the usual libc approach to set <code>errno</code> and return <code>-1</code>
instead, but goes with how the Linux kernel does it.</p>

<p class="block">Additional error codes may be returned as long as they are
negative. <i>libvxmdfmt</i> is an example, which has extra error codes in the
<code>-1500</code> range (<code>PWLFMT_E*</code>), outside the usual errno code
range.</p>

<p class="block">Applications must not assume that <code>errno</code> is set if
an error occurs. Library functions may diverge from this behavior to provide
additional error info, as e.g. <i>libvxmdfmt</i> which, if returning an error
code from its <code>PWLFMT_E*</code> range, has the underlying system error
code (e.g. <code>ENOENT</code>) in <code>errno</code> (which can/should be
used).</p>

<h2>zero</h2>

<p class="block">In case of Class B functions, this value is generally unused,
but counts as an error. A precise test for error therefore looks like:</p>

<p class="code"><code>
<b>int</b> ret;<br />
if((ret <b>=</b> function()) <b>&lt;=</b> 0)<br />
&nbsp; &nbsp; fprintf(stderr, "function() returned %s (%d)\n", strerror(-ret), ret);
</code></p>

<h2>positive non-zero</h2>

<p class="block">Any positive non-zero value indicates success.</p>

<h1>Class C&nbsp;-- integral value or error code</h1>

<h2>negative non-zero</h2>

<p class="block">For negative non-zero return values, the same rules as for
Class B apply.</p>

<h2>zero or positive non-zero</h2>

<p class="block">Zero or positive non-zero indicates the result of the
function. For example, asking <i>libvxdb</i> how many groups there are can
legitimately return zero:</p>

<p class="code"><code>
<b>long</b> ret;<br />
if((ret <b>=</b> vxdb_modctl(my_db, VXDB_COUNT_GROUPS)) <b>&gt;=</b> 0)<br />
&nbsp; &nbsp; printf("Number of groups: %ld\n", ret);<br />
else<br />
&nbsp; &nbsp; printf("vxdb_modctl() returned error %d: %s\n", ret, strerror(-ret));
</code></p>

<h1>Class D&nbsp;-- pointer or error code</h1>

<p class="block">Since any return value is used for the pointer, the only way
to notify of errors it to set <code>errno</code>. Therefore, if a functions
returns an invalid pointer (most likely <code>NULL</code>), <code>errno</code>
will be set.</p>

<?php include_once("Base-footer.php"); ?>
