#include "var.h"

#include "log.h"
#include "value.h"

UndefinedVar kUndefinedBuf;
UndefinedVar* kUndefined = &kUndefinedBuf;

Var::Var() {
}

Var::~Var() {
}

void Var::AppendVar(Evaluator*, Value*) {
  CHECK(false);
}

SimpleVar::SimpleVar(shared_ptr<string> v, const char* origin)
    : v_(v), origin_(origin) {
}

void SimpleVar::Eval(Evaluator*, string* s) const {
  *s += *v_;
}

void SimpleVar::AppendVar(Evaluator* ev, Value* v) {
  shared_ptr<string> s = make_shared<string>(*v_);
  s->push_back(' ');
  v->Eval(ev, s.get());
  v_ = s;
}

string SimpleVar::DebugString() const {
  return *v_;
}

RecursiveVar::RecursiveVar(Value* v, const char* origin)
    : v_(v), origin_(origin) {
}

void RecursiveVar::Eval(Evaluator* ev, string* s) const {
  v_->Eval(ev, s);
}

void RecursiveVar::AppendVar(Evaluator*, Value* v) {
  v_ = NewExpr3(v_, NewLiteral(" "), v);
}

string RecursiveVar::DebugString() const {
  return v_->DebugString();
}

UndefinedVar::UndefinedVar() {}

void UndefinedVar::Eval(Evaluator*, string*) const {
  // Nothing to do.
}

string UndefinedVar::DebugString() const {
  return "*undefined*";
}

Vars::~Vars() {
  for (auto p : *this) {
    delete p.second;
  }
}

Var* Vars::Lookup(StringPiece name) const {
  auto found = find(name);
  if (found == end())
    return kUndefined;
  return found->second;
}

void Vars::Assign(StringPiece name, Var* v) {
  auto p = insert(make_pair(name, v));
  if (!p.second) {
    if (p.first->second->IsDefined())
      delete p.first->second;
    p.first->second = v;
  }
}

ScopedVar::ScopedVar(Vars* vars, StringPiece name, Var* var)
    : vars_(vars), orig_(NULL) {
  auto p = vars->insert(make_pair(name, var));
  iter_ = p.first;
  if (!p.second) {
    orig_ = iter_->second;
    iter_->second = var;
  }
}

ScopedVar::~ScopedVar() {
  if (orig_) {
    iter_->second = orig_;
  } else {
    vars_->erase(iter_);
  }
}
