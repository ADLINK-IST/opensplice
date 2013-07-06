#!/bin/bash

function run_dds1578 ()
{
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/services/networking/dds1578" || return 1

    "$(dirname "$0")/../.cp_chmod" "$OSPL_HOME/etc/ospl.xml" ospl1578.xml || return 1
    patch -p0 <ospl_xml_for_dds1578.diff

    local script_dir="$PWD"

    cat "$OSPL_OUTER_HOME/testsuite/rbt/services/networking/dds1578/README" || return 1

    cd "$OSPL_OUTER_HOME/testsuite/rbt/services/networking/dds1578" || return 1
    OSPL_URI="file://$script_dir/ospl1578.xml" /bin/bash

    rm -f "$script_dir/ospl1578.xml"
}

function run_dds1513 ()
{
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/ccpp/common" || return 1
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/ccpp/multiDomain-common" || return 1
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/ccpp/multiDomain-domainBridge" || return 1

    cat "$OSPL_OUTER_HOME/testsuite/rbt/ccpp/multiDomain-domainBridge/bin/run_networked" || return 1

    cd "$OSPL_OUTER_HOME/testsuite/rbt/ccpp/multiDomain-domainBridge" || return 1
    /bin/bash
}

function run_dds2237 ()
{
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/saj/dds2237" || return 1

    echo "We don't run 2237 normally but if you want run bin/run_test"

    cd "$OSPL_OUTER_HOME/testsuite/rbt/saj/dds2237" || return 1
    /bin/bash
}

function run_dds2117 ()
{
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/services/networking/dds2117" || return 1

    cat "$OSPL_OUTER_HOME/testsuite/rbt/services/networking/dds2117/Readme.txt" || return 1

    cd "$OSPL_OUTER_HOME/testsuite/rbt/services/networking/dds2117" || return 1
    OSPL_URI="file://$PWD/dds2117.xml" /bin/bash
}

function run_dds1617 ()
{
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/sac/common" || return 1
    make -C "$OSPL_OUTER_HOME/testsuite/rbt/sac/sac_dds1617_CreatePersistentSnapshot" || return 1

    cat "$OSPL_OUTER_HOME/testsuite/rbt/sac/sac_dds1617_CreatePersistentSnapshot/eFDPfi_MW_DDS29/README" || return 1

    cd "$OSPL_OUTER_HOME/testsuite/rbt/sac/sac_dds1617_CreatePersistentSnapshot/eFDPfi_MW_DDS29" || return 1
    OSPL_URI="file://$PWD/sac_dds1617_CreatePersistentSnapshot.xml" /bin/bash
}

function run_eFDPfi_MW_SUP_33 ()
{
    make -C "$OSPL_HOME/../osplo/testsuite/rbt/tools/ospl/ospl_exit_codes_2/eFDPfi_MW_SUP_33_restart" || return 1

    echo "Run run_subscribers.sh on one host and then run_publishers.sh on another host"

    cd "$OSPL_OUTER_HOME/testsuite/rbt/tools/ospl/ospl_exit_codes_2/eFDPfi_MW_SUP_33_restart" || return 1
    /bin/bash

    make -C "$OSPL_OUTER_HOME/testsuite/rbt/tools/ospl/ospl_exit_codes_2/eFDPfi_MW_SUP_33_systemhalt" || return 1

    echo "Run run.sh"

    cd "$OSPL_OUTER_HOME/testsuite/rbt/tools/ospl/ospl_exit_codes_2/eFDPfi_MW_SUP_33_systemhalt" || return 1
    /bin/bash
}

function run_dds2466 ()
{
    cd "$OSPL_HOME/testsuite/tests/dds2466/" || return 1
    bash ./build.sh || return 1

    cat README || return 1

    /bin/bash
}

run_dds1578 || exit 1
run_dds1513 || exit 1
run_dds2237 || exit 1
if [ "$(uname -o)" != 'Cygwin' ]; then
    run_dds2117 || exit 1
    run_dds1617 || exit 1
    run_eFDPfi_MW_SUP_33 || exit 1
    run_dds2466 || exit 1
fi
