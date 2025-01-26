--
-- PostgreSQL database dump
--

-- Dumped from database version 16.3
-- Dumped by pg_dump version 16.3

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: notes; Type: TABLE; Schema: public; Owner: kerl
--

CREATE TABLE public.notes (
    id integer NOT NULL,
    section_id integer,
    body text NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.notes OWNER TO kerl;

--
-- Name: notes_id_seq; Type: SEQUENCE; Schema: public; Owner: kerl
--

CREATE SEQUENCE public.notes_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.notes_id_seq OWNER TO kerl;

--
-- Name: notes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: kerl
--

ALTER SEQUENCE public.notes_id_seq OWNED BY public.notes.id;


--
-- Name: sections; Type: TABLE; Schema: public; Owner: kerl
--

CREATE TABLE public.sections (
    id integer NOT NULL,
    name text NOT NULL,
    ancestor integer
);


ALTER TABLE public.sections OWNER TO kerl;

--
-- Name: sections_id_seq; Type: SEQUENCE; Schema: public; Owner: kerl
--

CREATE SEQUENCE public.sections_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.sections_id_seq OWNER TO kerl;

--
-- Name: sections_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: kerl
--

ALTER SEQUENCE public.sections_id_seq OWNED BY public.sections.id;


--
-- Name: notes id; Type: DEFAULT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.notes ALTER COLUMN id SET DEFAULT nextval('public.notes_id_seq'::regclass);


--
-- Name: sections id; Type: DEFAULT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.sections ALTER COLUMN id SET DEFAULT nextval('public.sections_id_seq'::regclass);


--
-- Name: notes name_uniq; Type: CONSTRAINT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.notes
    ADD CONSTRAINT name_uniq UNIQUE (name);


--
-- Name: notes notes_pkey; Type: CONSTRAINT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.notes
    ADD CONSTRAINT notes_pkey PRIMARY KEY (id);


--
-- Name: sections section_name; Type: CONSTRAINT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.sections
    ADD CONSTRAINT section_name UNIQUE (name);


--
-- Name: sections sections_pkey; Type: CONSTRAINT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.sections
    ADD CONSTRAINT sections_pkey PRIMARY KEY (id);


--
-- Name: notes section_note_fk; Type: FK CONSTRAINT; Schema: public; Owner: kerl
--

ALTER TABLE ONLY public.notes
    ADD CONSTRAINT section_note_fk FOREIGN KEY (section_id) REFERENCES public.sections(id);


--
-- PostgreSQL database dump complete
--

