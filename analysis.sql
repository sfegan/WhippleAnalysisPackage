CREATE SEQUENCE "file_id_seq" start 1 increment 1 maxvalue 2147483647 minvalue 1  cache 1 ;
CREATE SEQUENCE "compartment_id_seq" start 1 increment 1 maxvalue 2147483647 minvalue 1  cache 1 ;
CREATE TABLE "compartments" (
	"name" character varying(40) NOT NULL,
	"compartment_id" int4 NOT NULL,
	PRIMARY KEY ("name")
);
CREATE TABLE "data_file" (
	"name" character varying(40) NOT NULL,
	"utc_date" date NOT NULL,
	"compartment_id" int4 NOT NULL,
	"file_id" int4 NOT NULL,
	PRIMARY KEY ("name", "utc_date", "compartment_id")
);
CREATE TABLE "file_channel" (
	"file_id" int4 NOT NULL,
	"channel_num" int4 NOT NULL,
	PRIMARY KEY ("file_id", "channel_num")
);
CREATE TABLE "channel_data" (
	"val" float8 NOT NULL,
	"dev" float8 NOT NULL
)
inherits ("file_channel");
CREATE TABLE "peds_data" (
	
)
inherits ("channel_data");
CREATE TABLE "gains_data" (
	
)
inherits ("channel_data");
CREATE TABLE "peds_mask_data" (
	"reason" character varying(30) NOT NULL
)
inherits ("file_channel");
CREATE TABLE "gains_mask_data" (
	"reason" character varying(30) NOT NULL
)
inherits ("file_channel");
CREATE TABLE "pedsandgainsbase" (
	"file_id" int4 NOT NULL,
	"camera_name" character varying(40) NOT NULL,
	"nchannels" int4 NOT NULL,
	"nevents" int4 NOT NULL,
	"comment" character varying(80),
	PRIMARY KEY ("file_id")
);
CREATE TABLE "pedestals" (
	
)
inherits ("pedsandgainsbase");
CREATE TABLE "gains" (
	"mean_signal_mean" float8 NOT NULL,
	"mean_signal_dev" float8 NOT NULL
)
inherits ("pedsandgainsbase");
CREATE UNIQUE INDEX "compartments_compartment_id_key" on "compartments" using btree ( "compartment_id" "int4_ops" );
CREATE UNIQUE INDEX "data_file_file_id_key" on "data_file" using btree ( "file_id" "int4_ops" );
CREATE UNIQUE INDEX "pedestals_id_pkey" on "pedestals" using btree ( "file_id" "int4_ops" );
CREATE UNIQUE INDEX "gains_id_pkey" on "gains" using btree ( "file_id" "int4_ops" );
CREATE UNIQUE INDEX "gains_data_pkey" on "gains_data" using btree ( "file_id" "int4_ops", "channel_num" "int4_ops" );
CREATE UNIQUE INDEX "peds_data_pkey" on "peds_data" using btree ( "file_id" "int4_ops", "channel_num" "int4_ops" );
CREATE  INDEX "peds_mask_data_file_id_key" on "peds_mask_data" using btree ( "file_id" "int4_ops" );
CREATE  INDEX "gains_mask_data_file_id_key" on "gains_mask_data" using btree ( "file_id" "int4_ops" );
CREATE  INDEX "peds_data_file_id_key" on "peds_data" using btree ( "file_id" "int4_ops" );
CREATE  INDEX "gains_data_file_id_key" on "gains_data" using btree ( "file_id" "int4_ops" );
