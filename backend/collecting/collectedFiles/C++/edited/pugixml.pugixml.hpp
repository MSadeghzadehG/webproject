

#ifndef PUGIXML_VERSION
#	define PUGIXML_VERSION 180
#endif

#include "pugiconfig.hpp"

#ifndef HEADER_PUGIXML_HPP
#define HEADER_PUGIXML_HPP

#include <stddef.h>

#if !defined(PUGIXML_NO_XPATH) && !defined(PUGIXML_NO_EXCEPTIONS)
#	include <exception>
#endif

#ifndef PUGIXML_NO_STL
#	include <iterator>
#	include <iosfwd>
#	include <string>
#endif

#ifndef PUGIXML_DEPRECATED
#	if defined(__GNUC__)
#		define PUGIXML_DEPRECATED __attribute__((deprecated))
#	elif defined(_MSC_VER) && _MSC_VER >= 1300
#		define PUGIXML_DEPRECATED __declspec(deprecated)
#	else
#		define PUGIXML_DEPRECATED
#	endif
#endif

#ifndef PUGIXML_API
#	define PUGIXML_API
#endif

#ifndef PUGIXML_CLASS
#	define PUGIXML_CLASS PUGIXML_API
#endif

#ifndef PUGIXML_FUNCTION
#	define PUGIXML_FUNCTION PUGIXML_API
#endif

#ifndef PUGIXML_HAS_LONG_LONG
#	if __cplusplus >= 201103
#		define PUGIXML_HAS_LONG_LONG
#	elif defined(_MSC_VER) && _MSC_VER >= 1400
#		define PUGIXML_HAS_LONG_LONG
#	endif
#endif

#ifndef PUGIXML_HAS_MOVE
#	if __cplusplus >= 201103
#		define PUGIXML_HAS_MOVE
#	elif defined(_MSC_VER) && _MSC_VER >= 1600
#		define PUGIXML_HAS_MOVE
#	endif
#endif

#ifndef PUGIXML_OVERRIDE
#	if __cplusplus >= 201103
#		define PUGIXML_OVERRIDE override
#	else
#		define PUGIXML_OVERRIDE
#	endif
#endif

#ifdef PUGIXML_WCHAR_MODE
#	define PUGIXML_TEXT(t) L ## t
#	define PUGIXML_CHAR wchar_t
#else
#	define PUGIXML_TEXT(t) t
#	define PUGIXML_CHAR char
#endif

namespace pugi
{
		typedef PUGIXML_CHAR char_t;

#ifndef PUGIXML_NO_STL
		typedef std::basic_string<PUGIXML_CHAR, std::char_traits<PUGIXML_CHAR>, std::allocator<PUGIXML_CHAR> > string_t;
#endif
}

namespace pugi
{
		enum xml_node_type
	{
		node_null,					node_document,				node_element,				node_pcdata,				node_cdata,					node_comment,				node_pi,					node_declaration,			node_doctype			};

	
			const unsigned int parse_minimal = 0x0000;

		const unsigned int parse_pi = 0x0001;

		const unsigned int parse_comments = 0x0002;

		const unsigned int parse_cdata = 0x0004;

			const unsigned int parse_ws_pcdata = 0x0008;

		const unsigned int parse_escapes = 0x0010;

		const unsigned int parse_eol = 0x0020;

		const unsigned int parse_wconv_attribute = 0x0040;

		const unsigned int parse_wnorm_attribute = 0x0080;

		const unsigned int parse_declaration = 0x0100;

		const unsigned int parse_doctype = 0x0200;

				const unsigned int parse_ws_pcdata_single = 0x0400;

		const unsigned int parse_trim_pcdata = 0x0800;

			const unsigned int parse_fragment = 0x1000;

				const unsigned int parse_embed_pcdata = 0x2000;

				const unsigned int parse_default = parse_cdata | parse_escapes | parse_wconv_attribute | parse_eol;

				const unsigned int parse_full = parse_default | parse_pi | parse_comments | parse_declaration | parse_doctype;

		enum xml_encoding
	{
		encoding_auto,				encoding_utf8,				encoding_utf16_le,			encoding_utf16_be,			encoding_utf16,				encoding_utf32_le,			encoding_utf32_be,			encoding_utf32,				encoding_wchar,				encoding_latin1
	};

	
		const unsigned int format_indent = 0x01;

		const unsigned int format_write_bom = 0x02;

		const unsigned int format_raw = 0x04;

		const unsigned int format_no_declaration = 0x08;

		const unsigned int format_no_escapes = 0x10;

		const unsigned int format_save_file_text = 0x20;

		const unsigned int format_indent_attributes = 0x40;

		const unsigned int format_no_empty_element_tags = 0x80;

			const unsigned int format_default = format_indent;

		struct xml_attribute_struct;
	struct xml_node_struct;

	class xml_node_iterator;
	class xml_attribute_iterator;
	class xml_named_node_iterator;

	class xml_tree_walker;

	struct xml_parse_result;

	class xml_node;

	class xml_text;

	#ifndef PUGIXML_NO_XPATH
	class xpath_node;
	class xpath_node_set;
	class xpath_query;
	class xpath_variable_set;
	#endif

		template <typename It> class xml_object_range
	{
	public:
		typedef It const_iterator;
		typedef It iterator;

		xml_object_range(It b, It e): _begin(b), _end(e)
		{
		}

		It begin() const { return _begin; }
		It end() const { return _end; }

	private:
		It _begin, _end;
	};

		class PUGIXML_CLASS xml_writer
	{
	public:
		virtual ~xml_writer() {}

				virtual void write(const void* data, size_t size) = 0;
	};

		class PUGIXML_CLASS xml_writer_file: public xml_writer
	{
	public:
				xml_writer_file(void* file);

		virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

	private:
		void* file;
	};

	#ifndef PUGIXML_NO_STL
		class PUGIXML_CLASS xml_writer_stream: public xml_writer
	{
	public:
				xml_writer_stream(std::basic_ostream<char, std::char_traits<char> >& stream);
		xml_writer_stream(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream);

		virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

	private:
		std::basic_ostream<char, std::char_traits<char> >* narrow_stream;
		std::basic_ostream<wchar_t, std::char_traits<wchar_t> >* wide_stream;
	};
	#endif

		class PUGIXML_CLASS xml_attribute
	{
		friend class xml_attribute_iterator;
		friend class xml_node;

	private:
		xml_attribute_struct* _attr;

		typedef void (*unspecified_bool_type)(xml_attribute***);

	public:
				xml_attribute();

				explicit xml_attribute(xml_attribute_struct* attr);

				operator unspecified_bool_type() const;

				bool operator!() const;

				bool operator==(const xml_attribute& r) const;
		bool operator!=(const xml_attribute& r) const;
		bool operator<(const xml_attribute& r) const;
		bool operator>(const xml_attribute& r) const;
		bool operator<=(const xml_attribute& r) const;
		bool operator>=(const xml_attribute& r) const;

				bool empty() const;

				const char_t* name() const;
		const char_t* value() const;

				const char_t* as_string(const char_t* def = PUGIXML_TEXT("")) const;

				int as_int(int def = 0) const;
		unsigned int as_uint(unsigned int def = 0) const;
		double as_double(double def = 0) const;
		float as_float(float def = 0) const;

	#ifdef PUGIXML_HAS_LONG_LONG
		long long as_llong(long long def = 0) const;
		unsigned long long as_ullong(unsigned long long def = 0) const;
	#endif

				bool as_bool(bool def = false) const;

				bool set_name(const char_t* rhs);
		bool set_value(const char_t* rhs);

				bool set_value(int rhs);
		bool set_value(unsigned int rhs);
		bool set_value(long rhs);
		bool set_value(unsigned long rhs);
		bool set_value(double rhs);
		bool set_value(float rhs);
		bool set_value(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		bool set_value(long long rhs);
		bool set_value(unsigned long long rhs);
	#endif

				xml_attribute& operator=(const char_t* rhs);
		xml_attribute& operator=(int rhs);
		xml_attribute& operator=(unsigned int rhs);
		xml_attribute& operator=(long rhs);
		xml_attribute& operator=(unsigned long rhs);
		xml_attribute& operator=(double rhs);
		xml_attribute& operator=(float rhs);
		xml_attribute& operator=(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		xml_attribute& operator=(long long rhs);
		xml_attribute& operator=(unsigned long long rhs);
	#endif

				xml_attribute next_attribute() const;
		xml_attribute previous_attribute() const;

				size_t hash_value() const;

				xml_attribute_struct* internal_object() const;
	};

#ifdef __BORLANDC__
		bool PUGIXML_FUNCTION operator&&(const xml_attribute& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xml_attribute& lhs, bool rhs);
#endif

		class PUGIXML_CLASS xml_node
	{
		friend class xml_attribute_iterator;
		friend class xml_node_iterator;
		friend class xml_named_node_iterator;

	protected:
		xml_node_struct* _root;

		typedef void (*unspecified_bool_type)(xml_node***);

	public:
				xml_node();

				explicit xml_node(xml_node_struct* p);

				operator unspecified_bool_type() const;

				bool operator!() const;

				bool operator==(const xml_node& r) const;
		bool operator!=(const xml_node& r) const;
		bool operator<(const xml_node& r) const;
		bool operator>(const xml_node& r) const;
		bool operator<=(const xml_node& r) const;
		bool operator>=(const xml_node& r) const;

				bool empty() const;

				xml_node_type type() const;

				const char_t* name() const;

						const char_t* value() const;

				xml_attribute first_attribute() const;
		xml_attribute last_attribute() const;

				xml_node first_child() const;
		xml_node last_child() const;

				xml_node next_sibling() const;
		xml_node previous_sibling() const;

				xml_node parent() const;

				xml_node root() const;

				xml_text text() const;

				xml_node child(const char_t* name) const;
		xml_attribute attribute(const char_t* name) const;
		xml_node next_sibling(const char_t* name) const;
		xml_node previous_sibling(const char_t* name) const;

				xml_attribute attribute(const char_t* name, xml_attribute& hint) const;

				const char_t* child_value() const;

				const char_t* child_value(const char_t* name) const;

				bool set_name(const char_t* rhs);
		bool set_value(const char_t* rhs);

				xml_attribute append_attribute(const char_t* name);
		xml_attribute prepend_attribute(const char_t* name);
		xml_attribute insert_attribute_after(const char_t* name, const xml_attribute& attr);
		xml_attribute insert_attribute_before(const char_t* name, const xml_attribute& attr);

				xml_attribute append_copy(const xml_attribute& proto);
		xml_attribute prepend_copy(const xml_attribute& proto);
		xml_attribute insert_copy_after(const xml_attribute& proto, const xml_attribute& attr);
		xml_attribute insert_copy_before(const xml_attribute& proto, const xml_attribute& attr);

				xml_node append_child(xml_node_type type = node_element);
		xml_node prepend_child(xml_node_type type = node_element);
		xml_node insert_child_after(xml_node_type type, const xml_node& node);
		xml_node insert_child_before(xml_node_type type, const xml_node& node);

				xml_node append_child(const char_t* name);
		xml_node prepend_child(const char_t* name);
		xml_node insert_child_after(const char_t* name, const xml_node& node);
		xml_node insert_child_before(const char_t* name, const xml_node& node);

				xml_node append_copy(const xml_node& proto);
		xml_node prepend_copy(const xml_node& proto);
		xml_node insert_copy_after(const xml_node& proto, const xml_node& node);
		xml_node insert_copy_before(const xml_node& proto, const xml_node& node);

				xml_node append_move(const xml_node& moved);
		xml_node prepend_move(const xml_node& moved);
		xml_node insert_move_after(const xml_node& moved, const xml_node& node);
		xml_node insert_move_before(const xml_node& moved, const xml_node& node);

				bool remove_attribute(const xml_attribute& a);
		bool remove_attribute(const char_t* name);

				bool remove_child(const xml_node& n);
		bool remove_child(const char_t* name);

								xml_parse_result append_buffer(const void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

				template <typename Predicate> xml_attribute find_attribute(Predicate pred) const
		{
			if (!_root) return xml_attribute();

			for (xml_attribute attrib = first_attribute(); attrib; attrib = attrib.next_attribute())
				if (pred(attrib))
					return attrib;

			return xml_attribute();
		}

				template <typename Predicate> xml_node find_child(Predicate pred) const
		{
			if (!_root) return xml_node();

			for (xml_node node = first_child(); node; node = node.next_sibling())
				if (pred(node))
					return node;

			return xml_node();
		}

				template <typename Predicate> xml_node find_node(Predicate pred) const
		{
			if (!_root) return xml_node();

			xml_node cur = first_child();

			while (cur._root && cur._root != _root)
			{
				if (pred(cur)) return cur;

				if (cur.first_child()) cur = cur.first_child();
				else if (cur.next_sibling()) cur = cur.next_sibling();
				else
				{
					while (!cur.next_sibling() && cur._root != _root) cur = cur.parent();

					if (cur._root != _root) cur = cur.next_sibling();
				}
			}

			return xml_node();
		}

				xml_node find_child_by_attribute(const char_t* name, const char_t* attr_name, const char_t* attr_value) const;
		xml_node find_child_by_attribute(const char_t* attr_name, const char_t* attr_value) const;

	#ifndef PUGIXML_NO_STL
				string_t path(char_t delimiter = '/') const;
	#endif

				xml_node first_element_by_path(const char_t* path, char_t delimiter = '/') const;

				bool traverse(xml_tree_walker& walker);

	#ifndef PUGIXML_NO_XPATH
				xpath_node select_node(const char_t* query, xpath_variable_set* variables = 0) const;
		xpath_node select_node(const xpath_query& query) const;

				xpath_node_set select_nodes(const char_t* query, xpath_variable_set* variables = 0) const;
		xpath_node_set select_nodes(const xpath_query& query) const;

				xpath_node select_single_node(const char_t* query, xpath_variable_set* variables = 0) const;
		xpath_node select_single_node(const xpath_query& query) const;

	#endif

				void print(xml_writer& writer, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto, unsigned int depth = 0) const;

	#ifndef PUGIXML_NO_STL
				void print(std::basic_ostream<char, std::char_traits<char> >& os, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto, unsigned int depth = 0) const;
		void print(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& os, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, unsigned int depth = 0) const;
	#endif

				typedef xml_node_iterator iterator;

		iterator begin() const;
		iterator end() const;

				typedef xml_attribute_iterator attribute_iterator;

		attribute_iterator attributes_begin() const;
		attribute_iterator attributes_end() const;

				xml_object_range<xml_node_iterator> children() const;
		xml_object_range<xml_named_node_iterator> children(const char_t* name) const;
		xml_object_range<xml_attribute_iterator> attributes() const;

				ptrdiff_t offset_debug() const;

				size_t hash_value() const;

				xml_node_struct* internal_object() const;
	};

#ifdef __BORLANDC__
		bool PUGIXML_FUNCTION operator&&(const xml_node& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xml_node& lhs, bool rhs);
#endif

		class PUGIXML_CLASS xml_text
	{
		friend class xml_node;

		xml_node_struct* _root;

		typedef void (*unspecified_bool_type)(xml_text***);

		explicit xml_text(xml_node_struct* root);

		xml_node_struct* _data_new();
		xml_node_struct* _data() const;

	public:
				xml_text();

				operator unspecified_bool_type() const;

				bool operator!() const;

				bool empty() const;

				const char_t* get() const;

				const char_t* as_string(const char_t* def = PUGIXML_TEXT("")) const;

				int as_int(int def = 0) const;
		unsigned int as_uint(unsigned int def = 0) const;
		double as_double(double def = 0) const;
		float as_float(float def = 0) const;

	#ifdef PUGIXML_HAS_LONG_LONG
		long long as_llong(long long def = 0) const;
		unsigned long long as_ullong(unsigned long long def = 0) const;
	#endif

				bool as_bool(bool def = false) const;

				bool set(const char_t* rhs);

				bool set(int rhs);
		bool set(unsigned int rhs);
		bool set(long rhs);
		bool set(unsigned long rhs);
		bool set(double rhs);
		bool set(float rhs);
		bool set(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		bool set(long long rhs);
		bool set(unsigned long long rhs);
	#endif

				xml_text& operator=(const char_t* rhs);
		xml_text& operator=(int rhs);
		xml_text& operator=(unsigned int rhs);
		xml_text& operator=(long rhs);
		xml_text& operator=(unsigned long rhs);
		xml_text& operator=(double rhs);
		xml_text& operator=(float rhs);
		xml_text& operator=(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		xml_text& operator=(long long rhs);
		xml_text& operator=(unsigned long long rhs);
	#endif

				xml_node data() const;
	};

#ifdef __BORLANDC__
		bool PUGIXML_FUNCTION operator&&(const xml_text& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xml_text& lhs, bool rhs);
#endif

		class PUGIXML_CLASS xml_node_iterator
	{
		friend class xml_node;

	private:
		mutable xml_node _wrap;
		xml_node _parent;

		xml_node_iterator(xml_node_struct* ref, xml_node_struct* parent);

	public:
				typedef ptrdiff_t difference_type;
		typedef xml_node value_type;
		typedef xml_node* pointer;
		typedef xml_node& reference;

	#ifndef PUGIXML_NO_STL
		typedef std::bidirectional_iterator_tag iterator_category;
	#endif

				xml_node_iterator();

				xml_node_iterator(const xml_node& node);

				bool operator==(const xml_node_iterator& rhs) const;
		bool operator!=(const xml_node_iterator& rhs) const;

		xml_node& operator*() const;
		xml_node* operator->() const;

		const xml_node_iterator& operator++();
		xml_node_iterator operator++(int);

		const xml_node_iterator& operator--();
		xml_node_iterator operator--(int);
	};

		class PUGIXML_CLASS xml_attribute_iterator
	{
		friend class xml_node;

	private:
		mutable xml_attribute _wrap;
		xml_node _parent;

		xml_attribute_iterator(xml_attribute_struct* ref, xml_node_struct* parent);

	public:
				typedef ptrdiff_t difference_type;
		typedef xml_attribute value_type;
		typedef xml_attribute* pointer;
		typedef xml_attribute& reference;

	#ifndef PUGIXML_NO_STL
		typedef std::bidirectional_iterator_tag iterator_category;
	#endif

				xml_attribute_iterator();

				xml_attribute_iterator(const xml_attribute& attr, const xml_node& parent);

				bool operator==(const xml_attribute_iterator& rhs) const;
		bool operator!=(const xml_attribute_iterator& rhs) const;

		xml_attribute& operator*() const;
		xml_attribute* operator->() const;

		const xml_attribute_iterator& operator++();
		xml_attribute_iterator operator++(int);

		const xml_attribute_iterator& operator--();
		xml_attribute_iterator operator--(int);
	};

		class PUGIXML_CLASS xml_named_node_iterator
	{
		friend class xml_node;

	public:
				typedef ptrdiff_t difference_type;
		typedef xml_node value_type;
		typedef xml_node* pointer;
		typedef xml_node& reference;

	#ifndef PUGIXML_NO_STL
		typedef std::bidirectional_iterator_tag iterator_category;
	#endif

				xml_named_node_iterator();

				xml_named_node_iterator(const xml_node& node, const char_t* name);

				bool operator==(const xml_named_node_iterator& rhs) const;
		bool operator!=(const xml_named_node_iterator& rhs) const;

		xml_node& operator*() const;
		xml_node* operator->() const;

		const xml_named_node_iterator& operator++();
		xml_named_node_iterator operator++(int);

		const xml_named_node_iterator& operator--();
		xml_named_node_iterator operator--(int);

	private:
		mutable xml_node _wrap;
		xml_node _parent;
		const char_t* _name;

		xml_named_node_iterator(xml_node_struct* ref, xml_node_struct* parent, const char_t* name);
	};

		class PUGIXML_CLASS xml_tree_walker
	{
		friend class xml_node;

	private:
		int _depth;

	protected:
				int depth() const;

	public:
		xml_tree_walker();
		virtual ~xml_tree_walker();

				virtual bool begin(xml_node& node);

				virtual bool for_each(xml_node& node) = 0;

				virtual bool end(xml_node& node);
	};

		enum xml_parse_status
	{
		status_ok = 0,				
		status_file_not_found,				status_io_error,					status_out_of_memory,				status_internal_error,		
		status_unrecognized_tag,	
		status_bad_pi,						status_bad_comment,					status_bad_cdata,					status_bad_doctype,					status_bad_pcdata,					status_bad_start_element,			status_bad_attribute,				status_bad_end_element,				status_end_element_mismatch,
		status_append_invalid_root,	
		status_no_document_element		};

		struct PUGIXML_CLASS xml_parse_result
	{
				xml_parse_status status;

				ptrdiff_t offset;

				xml_encoding encoding;

				xml_parse_result();

				operator bool() const;

				const char* description() const;
	};

		class PUGIXML_CLASS xml_document: public xml_node
	{
	private:
		char_t* _buffer;

		char _memory[192];

				xml_document(const xml_document&);
		xml_document& operator=(const xml_document&);

		void _create();
		void _destroy();

	public:
				xml_document();

				~xml_document();

				void reset();

				void reset(const xml_document& proto);

	#ifndef PUGIXML_NO_STL
				xml_parse_result load(std::basic_istream<char, std::char_traits<char> >& stream, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);
		xml_parse_result load(std::basic_istream<wchar_t, std::char_traits<wchar_t> >& stream, unsigned int options = parse_default);
	#endif

				xml_parse_result load(const char_t* contents, unsigned int options = parse_default);

				xml_parse_result load_string(const char_t* contents, unsigned int options = parse_default);

				xml_parse_result load_file(const char* path, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);
		xml_parse_result load_file(const wchar_t* path, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

				xml_parse_result load_buffer(const void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

						xml_parse_result load_buffer_inplace(void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

						xml_parse_result load_buffer_inplace_own(void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

				void save(xml_writer& writer, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;

	#ifndef PUGIXML_NO_STL
				void save(std::basic_ostream<char, std::char_traits<char> >& stream, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;
		void save(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default) const;
	#endif

				bool save_file(const char* path, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;
		bool save_file(const wchar_t* path, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;

				xml_node document_element() const;
	};

#ifndef PUGIXML_NO_XPATH
		enum xpath_value_type
	{
		xpath_type_none,	  		xpath_type_node_set,  		xpath_type_number,	  		xpath_type_string,	  		xpath_type_boolean	  	};

		struct PUGIXML_CLASS xpath_parse_result
	{
				const char* error;

				ptrdiff_t offset;

				xpath_parse_result();

				operator bool() const;

				const char* description() const;
	};

		class PUGIXML_CLASS xpath_variable
	{
		friend class xpath_variable_set;

	protected:
		xpath_value_type _type;
		xpath_variable* _next;

		xpath_variable(xpath_value_type type);

				xpath_variable(const xpath_variable&);
		xpath_variable& operator=(const xpath_variable&);

	public:
				const char_t* name() const;

				xpath_value_type type() const;

				bool get_boolean() const;
		double get_number() const;
		const char_t* get_string() const;
		const xpath_node_set& get_node_set() const;

				bool set(bool value);
		bool set(double value);
		bool set(const char_t* value);
		bool set(const xpath_node_set& value);
	};

		class PUGIXML_CLASS xpath_variable_set
	{
	private:
		xpath_variable* _data[64];

		void _assign(const xpath_variable_set& rhs);
		void _swap(xpath_variable_set& rhs);

		xpath_variable* _find(const char_t* name) const;

		static bool _clone(xpath_variable* var, xpath_variable** out_result);
		static void _destroy(xpath_variable* var);

	public:
				xpath_variable_set();
		~xpath_variable_set();

				xpath_variable_set(const xpath_variable_set& rhs);
		xpath_variable_set& operator=(const xpath_variable_set& rhs);

	#ifdef PUGIXML_HAS_MOVE
				xpath_variable_set(xpath_variable_set&& rhs);
		xpath_variable_set& operator=(xpath_variable_set&& rhs);
	#endif

				xpath_variable* add(const char_t* name, xpath_value_type type);

				bool set(const char_t* name, bool value);
		bool set(const char_t* name, double value);
		bool set(const char_t* name, const char_t* value);
		bool set(const char_t* name, const xpath_node_set& value);

				xpath_variable* get(const char_t* name);
		const xpath_variable* get(const char_t* name) const;
	};

		class PUGIXML_CLASS xpath_query
	{
	private:
		void* _impl;
		xpath_parse_result _result;

		typedef void (*unspecified_bool_type)(xpath_query***);

				xpath_query(const xpath_query&);
		xpath_query& operator=(const xpath_query&);

	public:
						explicit xpath_query(const char_t* query, xpath_variable_set* variables = 0);

				xpath_query();

				~xpath_query();

	#ifdef PUGIXML_HAS_MOVE
				xpath_query(xpath_query&& rhs);
		xpath_query& operator=(xpath_query&& rhs);
	#endif

				xpath_value_type return_type() const;

						bool evaluate_boolean(const xpath_node& n) const;

						double evaluate_number(const xpath_node& n) const;

	#ifndef PUGIXML_NO_STL
						string_t evaluate_string(const xpath_node& n) const;
	#endif

										size_t evaluate_string(char_t* buffer, size_t capacity, const xpath_node& n) const;

								xpath_node_set evaluate_node_set(const xpath_node& n) const;

										xpath_node evaluate_node(const xpath_node& n) const;

				const xpath_parse_result& result() const;

				operator unspecified_bool_type() const;

				bool operator!() const;
	};

	#ifndef PUGIXML_NO_EXCEPTIONS
		class PUGIXML_CLASS xpath_exception: public std::exception
	{
	private:
		xpath_parse_result _result;

	public:
				explicit xpath_exception(const xpath_parse_result& result);

				virtual const char* what() const throw() PUGIXML_OVERRIDE;

				const xpath_parse_result& result() const;
	};
	#endif

		class PUGIXML_CLASS xpath_node
	{
	private:
		xml_node _node;
		xml_attribute _attribute;

		typedef void (*unspecified_bool_type)(xpath_node***);

	public:
				xpath_node();

				xpath_node(const xml_node& node);
		xpath_node(const xml_attribute& attribute, const xml_node& parent);

				xml_node node() const;
		xml_attribute attribute() const;

				xml_node parent() const;

				operator unspecified_bool_type() const;

				bool operator!() const;

				bool operator==(const xpath_node& n) const;
		bool operator!=(const xpath_node& n) const;
	};

#ifdef __BORLANDC__
		bool PUGIXML_FUNCTION operator&&(const xpath_node& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xpath_node& lhs, bool rhs);
#endif

		class PUGIXML_CLASS xpath_node_set
	{
	public:
				enum type_t
		{
			type_unsorted,						type_sorted,						type_sorted_reverse				};

				typedef const xpath_node* const_iterator;

				typedef const xpath_node* iterator;

				xpath_node_set();

				xpath_node_set(const_iterator begin, const_iterator end, type_t type = type_unsorted);

				~xpath_node_set();

				xpath_node_set(const xpath_node_set& ns);
		xpath_node_set& operator=(const xpath_node_set& ns);

	#ifdef PUGIXML_HAS_MOVE
				xpath_node_set(xpath_node_set&& rhs);
		xpath_node_set& operator=(xpath_node_set&& rhs);
	#endif

				type_t type() const;

				size_t size() const;

				const xpath_node& operator[](size_t index) const;

				const_iterator begin() const;
		const_iterator end() const;

				void sort(bool reverse = false);

				xpath_node first() const;

				bool empty() const;

	private:
		type_t _type;

		xpath_node _storage;

		xpath_node* _begin;
		xpath_node* _end;

		void _assign(const_iterator begin, const_iterator end, type_t type);
		void _move(xpath_node_set& rhs);
	};
#endif

#ifndef PUGIXML_NO_STL
		std::basic_string<char, std::char_traits<char>, std::allocator<char> > PUGIXML_FUNCTION as_utf8(const wchar_t* str);
	std::basic_string<char, std::char_traits<char>, std::allocator<char> > PUGIXML_FUNCTION as_utf8(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >& str);

		std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > PUGIXML_FUNCTION as_wide(const char* str);
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > PUGIXML_FUNCTION as_wide(const std::basic_string<char, std::char_traits<char>, std::allocator<char> >& str);
#endif

		typedef void* (*allocation_function)(size_t size);

		typedef void (*deallocation_function)(void* ptr);

		void PUGIXML_FUNCTION set_memory_management_functions(allocation_function allocate, deallocation_function deallocate);

		allocation_function PUGIXML_FUNCTION get_memory_allocation_function();
	deallocation_function PUGIXML_FUNCTION get_memory_deallocation_function();
}

#if !defined(PUGIXML_NO_STL) && (defined(_MSC_VER) || defined(__ICC))
namespace std
{
		std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_node_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_attribute_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_named_node_iterator&);
}
#endif

#if !defined(PUGIXML_NO_STL) && defined(__SUNPRO_CC)
namespace std
{
		std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_node_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_attribute_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_named_node_iterator&);
}
#endif

#endif

#if defined(PUGIXML_HEADER_ONLY) && !defined(PUGIXML_SOURCE)
#	define PUGIXML_SOURCE "pugixml.cpp"
#	include PUGIXML_SOURCE
#endif


