# V3.0 menu cluster identity test

Compile this test against the built MU1102 VCMenu.jar, then run with that JAR and the test output on the classpath using `java -Xverify:all TestV3ClusterProfile`. It exercises the actual shipped helper without the offline cluster override. It checks dedicated profiles independently of product versions, matching/conflicting confirmation, missing input and genuine legacy identifiers. This pure-Java test does not simulate real touch or a headunit.
