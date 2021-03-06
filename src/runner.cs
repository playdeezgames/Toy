using System;
using System.IO;
using System.Text;
using System.Collections.Generic;

namespace Toy {
	class Runner {
		public static Environment RunFile(string filename) {
			Environment env = Run(File.ReadAllText(filename, Encoding.UTF8));

			if (ErrorHandler.HadError) {
				return null;
			}

			return env;
		}

		public static Environment RunFile(Environment environment, string filename) {
			Environment env = Run(environment, File.ReadAllText(filename, Encoding.UTF8));

			if (ErrorHandler.HadError) {
				return null;
			}

			return env;
		}

		public static Environment Run(string source) {
			return Run(new Environment(), source);
		}

		public static Environment Run(Environment env, string source) {
			return Run(new Interpreter(env == null ? new Environment() : env), source);
		}

		//call an ICallable object directly
		public static object Run(Environment env, ICallable callable, List<object> arguments) {
			try {
				//build the call object
				Token token = new Token(TokenType.EOF, "internal", null, -1);

				//build a new interpreter
				Interpreter interpreter = new Interpreter(env);
				Resolver resolver = new Resolver(interpreter);

				resolver.Resolve(((ScriptFunction)callable).GetDeclaration());

				//call the ICallable, returning the result
				return callable.Call(interpreter, token, arguments);

			//WARNING: duplicate code
			} catch(ErrorHandler.AssertError) {
				ConsoleOutput.Log("Assert error caught at Run()");
				ConsoleOutput.Log("The program will now exit early");
			} catch (ErrorHandler.ParserError e) {
				ConsoleOutput.Log("Parser error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			} catch (ErrorHandler.ResolverError e) {
				ConsoleOutput.Log("Resolver error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			} catch (ErrorHandler.RuntimeError e) {
				ConsoleOutput.Log("Runtime error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			} catch (Exception e) {
				ConsoleOutput.Log("Terminal error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			}

			return null;
		}

		//actually run the source code
		public static Environment Run(Interpreter interpreter, string source) {
			try {
				Lexer lexer = new Lexer(source);
				Parser parser = new Parser(lexer.ScanTokens());
				List<Stmt> stmtList = parser.ParseStatements();

				if (ErrorHandler.HadError) {
					return null;
				}

				Resolver resolver = new Resolver(interpreter);
				resolver.Resolve(stmtList);

				if (ErrorHandler.HadError) {
					return null;
				}

				interpreter.Interpret(stmtList);

				//return the environment context for the import keyword
				return interpreter.environment;

			} catch(ErrorHandler.AssertError) {
				ConsoleOutput.Log("Assert error caught at Run()");
				ConsoleOutput.Log("The program will now exit early");
			} catch (ErrorHandler.ParserError e) {
				ConsoleOutput.Log("Parser error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			} catch (ErrorHandler.ResolverError e) {
				ConsoleOutput.Log("Resolver error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			} catch (ErrorHandler.RuntimeError e) {
				ConsoleOutput.Log("Runtime error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			} catch (Exception e) {
				ConsoleOutput.Log("Terminal error caught at Run()");
				ConsoleOutput.Log("The following output is for internal debugging only, and will be removed from the final release:\n" + e.ToString());
			}

			return null;
		}
	}
}