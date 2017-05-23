DCPS Modules     {#DCPS_Modules}
============
The DCPS is comprised of four modules all of which are based on a \subpage DCPS_Modules_Infrastructure "Entity" abstract base class also known as the infrastructure module

- \subpage DCPS_Modules_DomainModule "Domain Module"
- \subpage DCPS_Modules_TopicDefinition "Topic-Definition Module"
- \subpage DCPS_Modules_Publication "Publication Module"
- \subpage DCPS_Modules_Subscription "Subscription Module"

\dot
digraph Modules
{
     En[label="Infrastructure Module",shape=box, URL="\ref DCPS_Modules_Entity"];
     D[label="Domain Module", shape=box, URL="\ref DCPS_Modules_DomainParticipant"];
     T[label="Topic-Definition Module",shape=box, URL="\ref DCPS_Modules_TopicDefinition"];
     P[label="Publication Module",shape=box, URL="\ref DCPS_Modules_Publication"];
     S[label="Subscription Module",shape=box, URL="\ref DCPS_Modules_Subscription"];

     D -> S -> T;
     D -> P -> T;
     T -> En;
}
\enddot