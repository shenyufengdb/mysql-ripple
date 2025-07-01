FROM centos:centos7.9.2009

# change the repo to aliyun
RUN ( cp -r /etc/yum.repos.d /etc/yum.repos.d.bak && \
    rm -f /etc/yum.repos.d/CentOS-Base.repo && \
    curl -o /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo && \
    yum install -y centos-release-scl && cd /etc/yum.repos.d && \
    mv CentOS-SCLo-scl.repo CentOS-SCLo-scl.repo-bak && \
    mv CentOS-SCLo-scl-rh.repo CentOS-SCLo-scl-rh.repo.bak && \
    echo "[centos-sclo-rh]" > CentOS-SCLo-scl-rh.repo && \
    echo "name=CentOS-7 - SCLo rh" >> CentOS-SCLo-scl-rh.repo && \
    echo "baseurl=https://mirrors.aliyun.com/centos/7/sclo/x86_64/rh/" >> CentOS-SCLo-scl-rh.repo && \
    echo "gpgcheck=1" >> CentOS-SCLo-scl-rh.repo && \
    echo "enabled=1" >> CentOS-SCLo-scl-rh.repo && \
    echo "gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-SIG-SCLo" >> CentOS-SCLo-scl-rh.repo \
)
RUN (mkdir -p /soft/mysql/build/tmp && chmod -R 777 /soft/mysql/build && chmod 777 /soft/mysql)
RUN (sed -i 's|root:x:0:0:root:/root:/bin/bash|root:x:0:0:root:/soft/mysql:/bin/bash|' /etc/passwd)
RUN (groupadd mysql)
RUN (useradd -l -g mysql -d /soft/mysql mysql)

RUN yum install -y epel-release cmake3 make openssl openssl-devel mysql-devel mysql-client devtoolset-8 && yum clean all && scl enable devtoolset-8 bash

RUN (yum -y install devtoolset-8-libasan-devel cmake3 libasan libasan5 perl perl-Digest-MD5 perl-Env \
                    autoconf automake libtool curl wget unzip  leveldb-devel ctags \
                    cscope  python3 python3-devel vim sudo \
                    perl-Data-Dumper perl-Test-Harness bison flex \
                    perl-JSON perl-Test-Simple perl-Capture-Tiny perl-TimeDate \
                    perl-DateTime perl-Module-Load perl-Module-Load-Conditional perl-JSON-XS \
                    libaio numactl net-tools iptables-services)


RUN (cd /soft/mysql/build/tmp && \
     wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-community-libs-5.7.25-1.el7.x86_64.rpm && \
     wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-community-common-5.7.25-1.el7.x86_64.rpm && \
     wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-community-client-5.7.25-1.el7.x86_64.rpm && \
     wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-community-devel-5.7.25-1.el7.x86_64.rpm && \
     wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-community-server-5.7.25-1.el7.x86_64.rpm && \
     wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-community-libs-compat-5.7.25-1.el7.x86_64.rpm && \
     yum remove -y mariadb-devel mariadb-libs && \
     rpm -ivh *.rpm && rm -f *.rpm)

WORKDIR /soft/mysql