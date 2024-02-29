class STATE_GERMANY {
public:
	enum STATE_GERMANY_TYPE {
		UNKNOWN=-1, GERMANY=0, NON_GERMANY=1
	};

	STATE_GERMANY_TYPE type;
	STATE_GERMANY() = default;
	constexpr STATE_GERMANY(STATE_GERMANY_TYPE _type) : type(_type) { }
	constexpr STATE_GERMANY(int _type) : type(static_cast<STATE_GERMANY_TYPE>(_type)) { }
	operator STATE_GERMANY_TYPE() const { return type; }
	constexpr bool operator == (STATE_GERMANY error) const { return type == error.type; }
	constexpr bool operator != (STATE_GERMANY error) const { return type != error.type; }    
	constexpr bool operator == (STATE_GERMANY_TYPE errorType) const { return type == errorType; }
	constexpr bool operator != (STATE_GERMANY_TYPE errorType) const { return type != errorType; }
	static const STATE_GERMANY unknown;
	static const STATE_GERMANY def;
	static const int size = 2;

};

const STATE_GERMANY STATE_GERMANY::unknown = STATE_GERMANY::STATE_GERMANY_TYPE::UNKNOWN;
const STATE_GERMANY STATE_GERMANY::def = STATE_GERMANY::STATE_GERMANY_TYPE::NON_GERMANY;


ostream& operator<<(ostream& os, STATE_GERMANY s) {
	return os << ((s.type == STATE_GERMANY::STATE_GERMANY_TYPE::GERMANY) ? "Germany" : (s.type == STATE_GERMANY::STATE_GERMANY_TYPE::UNKNOWN ? "U" : "nonGermany")) ;
}

istream& operator>>(istream& is, STATE_GERMANY& st) {
	string s;
	is >> s;
	if (s == "Germany")
		st.type = STATE_GERMANY::STATE_GERMANY_TYPE::GERMANY;
	else if (s == "nonGermany")
		st.type = STATE_GERMANY::STATE_GERMANY_TYPE::NON_GERMANY;
	else {
		assert(s == "unknown");
		st.type = STATE_GERMANY::STATE_GERMANY_TYPE::UNKNOWN;
	}
	return is;
}


class State_Dusseldorf {
public:
	enum State_Dusseldorf_Type {
		UNKNOWN=-1, DUSSELDORF=0, NON_DUSSELDORF=1
	};

	State_Dusseldorf_Type type;
	State_Dusseldorf() = default;
	constexpr State_Dusseldorf(State_Dusseldorf_Type _type) : type(_type) { }
	constexpr State_Dusseldorf(int _type) : type(static_cast<State_Dusseldorf_Type>(_type)) { }
	operator  State_Dusseldorf_Type() const { return type; }
	constexpr bool operator == (State_Dusseldorf error) const { return type == error.type; }
	constexpr bool operator != (State_Dusseldorf error) const { return type != error.type; }    
	constexpr bool operator == (State_Dusseldorf_Type errorType) const { return type == errorType; }
	constexpr bool operator != (State_Dusseldorf_Type errorType) const { return type != errorType; }
	static const State_Dusseldorf unknown;
	static const State_Dusseldorf def;
	static const int size = 2;

};

const State_Dusseldorf State_Dusseldorf::unknown = State_Dusseldorf::State_Dusseldorf_Type::UNKNOWN;
const State_Dusseldorf State_Dusseldorf::def = State_Dusseldorf::State_Dusseldorf_Type::NON_DUSSELDORF;


ostream& operator<<(ostream& os, State_Dusseldorf s) {
	return os << ((s.type == State_Dusseldorf::State_Dusseldorf_Type::DUSSELDORF) ? "Dusseldorf" : (s.type == State_Dusseldorf::State_Dusseldorf_Type::UNKNOWN ? "U" : "nonDusseldorf")) ;
}

istream& operator>>(istream& is, State_Dusseldorf& st) {
	string s;
	is >> s;
	if (s == "Dusseldorf")
		st.type = State_Dusseldorf::State_Dusseldorf_Type::DUSSELDORF;
	else if (s == "nonDusseldorf")
		st.type = State_Dusseldorf::State_Dusseldorf_Type::NON_DUSSELDORF;
	else {
		assert(s == "unknown");
		st.type = State_Dusseldorf::State_Dusseldorf_Type::UNKNOWN;
	}
	return is;
}

class StateInOut {
public:
	enum Type {
		UNKNOWN=-1, IN=0, OUT=1
	};

	Type type;
	StateInOut() = default;
	constexpr StateInOut(Type _type) : type(_type) { }
	constexpr StateInOut(int _type) : type(static_cast<Type>(_type)) { }
	operator  Type() const { return type; }
	constexpr bool operator == (StateInOut error) const { return type == error.type; }
	constexpr bool operator != (StateInOut error) const { return type != error.type; }    
	constexpr bool operator == (Type errorType) const { return type == errorType; }
	constexpr bool operator != (Type errorType) const { return type != errorType; }
	static const StateInOut unknown;
	static const StateInOut def;
	static const int size = 2;

	//IN, OUT
	static array<string, 2> names;
};

const StateInOut StateInOut::unknown = StateInOut::Type::UNKNOWN;
const StateInOut StateInOut::def = StateInOut::Type::OUT;
ostream& operator<<(ostream& os, StateInOut s) {
	return os << ((s.type == StateInOut::Type::IN) ? StateInOut::names[0] : (s.type == StateInOut::Type::UNKNOWN ? "U" : StateInOut::names[1])) ;
}

istream& operator>>(istream& is, StateInOut& st) {
	string s;
	is >> s;
	if (s == StateInOut::names[0])
		st.type = StateInOut::Type::IN;
	else if (s == StateInOut::names[1])
		st.type = StateInOut::Type::OUT;
	else {
		if (s != "unknown")
			cerr << "Invalid state " << s << " " << StateInOut::names[0] << " " << StateInOut::names[1] << endl;
		assert(s == "unknown");
		st.type = StateInOut::Type::UNKNOWN;
	}
	return is;
}

