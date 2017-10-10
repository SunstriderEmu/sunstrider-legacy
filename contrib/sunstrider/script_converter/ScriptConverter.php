<?php

class ScriptConverter
{
	private $content = array();
	
	function __construct($filepath)
	{
		$handle = fopen($filepath, "r");
		if($handle === false)
			throw new Exception("Could not open file for reading with path $filepath");
		
		while (($line = fgets($handle)) !== false) {
			array_push($this->content, $line);
		}
		fclose($handle);
		echo 'ScriptConverter: Constructed with ' . count($this->content) . ' lines.' .PHP_EOL;
	}
	
	public function Convert($out_file_path) : void
	{
		$ok = $this->_Convert();
		if(!$ok)
			echo "Failed to convert to file $out_file_path " .PHP_EOL;
		
		$res = file_put_contents($out_file_path, implode($this->content));
		if($res === false)
			throw new Exception("Could not write to file $out_file_path");
	}
	
	private function _Convert() : bool
	{
		$script_names = $this->DetectScriptsNames();
		if(empty($script_names)) {
			echo "Warning: Didn't find any script names for this file" . PHP_EOL;
			return false;
		}
		echo "Detected script with names: " . PHP_EOL;
		print_r($script_names);
		
		foreach($script_names as $script_name) {
			try {
				//init old script
				$script = new OldScript($script_name, $this->content);
				
				//remove old registering. Must be first because some lines are stored into the registering object
				echo "Replacing script registering part...".PHP_EOL;
				$this->ReplaceScriptRegistering($script, $script_name);
				
				//generate new script
				$script->ReplaceOldScript($this->content);
				echo "Finished handling $script_name " . PHP_EOL;
				//break;
			} catch (Exception $e) {
				echo "Failed to read script $script_name. Exception: " . $e->getMessage() . PHP_EOL.PHP_EOL;
			}
		}
		$total_replacements = $this->ReplaceClassCasts();
		if($total_replacements > 0)
			echo "Replaced $total_replacements class casts in file" . PHP_EOL.PHP_EOL;
		
		$this->RemoveOLDScriptDeclaration();
		echo PHP_EOL;
		
		return true;
	}
	
	//try to find cast such as (boss_kelidan_the_breakerAI*) and replace them with (boss_kelidan_the_breaker::boss_kelidan_the_breakerAI*)
	//(needed because AI is now nested in new script format)
	private function ReplaceClassCasts() : int
	{
		$total_count = 0;
		foreach($this->content as $key => &$line) {
			//exclude those
			if( strpos($line, "npc_escortAI") !== false 
			 || strpos($line, "CreatureAI") !== false
			 || strpos($line, "ScriptedAI") !== false )
				continue;
				
			$count = 0;
			$line = preg_replace('/\((\w+)AI ?\*\)/', '($1::$1AI*)', $line, -1, $count);
			$total_count += $count;
			$line = preg_replace('/CAST_AI\((\w+)AI\,/', 'CAST_AI($1::$1AI,', $line, -1, $count);
			$total_count += $count;
		}
		return $total_count;
	}
	
	private function RemoveOLDScriptDeclaration() : void
	{
		foreach($this->content as $key => $line) {
			if(   strpos($line, "OLDScript*") !== false 
			   || strpos($line, "OLDScript *") !== false ) {
				unset($this->content[$key]);
				return;
			}
		}
	}
	
	public static function RemoveLines(&$array, $from, $to) : void
	{
		$to_delete_count = $to - $from + 1;
		if($to_delete_count < 2) 
			throw new Exception("RemoveLines: From $from to $from, not supposed to happen");
		
		$line_count = 0;
		foreach($array as $key => $line) {
			if($line_count < $from) {
				$line_count++;
				continue;
			}
			
			if($to_delete_count <= 0)
				break;
			
			//delete this element
			unset($array[$key]);
			$to_delete_count--;
		}
	}
	
	public static function InsertFromLine(&$array, $from, array $script_lines) : void
	{
		$array = array_merge(
							array_slice($array, 0, $from, true),
							$script_lines,
							array_slice($array, $from, count($array) - 1, true)
						 );
	}
	
	/* return an array with script names found in file contents
	*/
	private function DetectScriptsNames() : array
	{
		$script_names = array();
		foreach($this->content as $line) {
			$matches = array(); 
			$found = preg_match('/[^ ]+->Name[ ]*=[ ]*"(.+)";/', $line, $matches);
			if($found)
				array_push($script_names, $matches[1]);
		}
		return $script_names;
	}
	
	//can throw
	private function ReplaceScriptRegistering(OldScript &$script, $script_name) : void
	{		
		$registering_boundaries = $script->GetRegistering()->GetBoundaries();
		$from = $registering_boundaries[0];
		$to = $registering_boundaries[1];
		
		$this->RemoveLines($this->content, $from, $to);
		$this->InsertFromLine($this->content, $from, array("    new $script_name();".PHP_EOL)); //assume 4 spaces... should be correct in most if not all cases
	}
}

/*
This classes handle finding all functions names in script registering.
*/
class OldScriptRegistering
{
	private $first_line = null;
	private $last_line = null;
	
	private $otherFunctions = array();
	private $getAIFunction = null;
	
	function __construct($script_name, array &$lines)
	{
		$line_number = 0;
		
		//find registering first line
		$last_new_oldscript_line_number = 0; //last line where we've seen a "new OLDScript"
		foreach($lines as &$line) {
			if(strpos($line, 'new OLDScript') !== false)
				$last_new_oldscript_line_number = $line_number;
			
			//look for script name. If we find it, memorize the line and stop there
			//echo "Trying to match line:".PHP_EOL.$line;
			$found = preg_match("/([^ ]+)->Name[ ]*=[ ]*\"$script_name\";/", $line);
			if($found === 1) {
				//we should have found a "new OLDScript" line
				if($last_new_oldscript_line_number === 0)
					throw new Exception("OldScriptRegistering: Could not find any new OLDscript line for $script_name");
				
				$this->first_line = $last_new_oldscript_line_number;
				break;
			}
				
			$line_number++;
		}
		
		//we should have found the name line
		if($this->first_line === null)
			throw new Exception("OldScriptRegistering: Could not find script name registering for script $script_name");
		
		//but name line may not be the first in the registering definitions so let's start from the last "new OLDScript" line
		$this->FindFunctions($lines, $this->first_line);
		$functionCount = count($this->otherFunctions) + ($this->getAIFunction === null ? 0 : 1);
		echo "OldScriptRegistering: Found a total of $functionCount functions in script registering" . PHP_EOL;
	}
	
	private function FindFunctions(&$lines, $from_line) : void
	{
		$line_number = 0;
		foreach($lines as $line) {
			if($line_number < $from_line) {
				$line_number++;
				continue;
			}
			
			//look for script name. If we find it, memorize the line and stop there
			$matches = array();
			$found = preg_match('/[^ ]+->(\w+)[ ]*=[ ]*&(\w+);/', $line, $matches);
			if($found === 1)
				$this->RegisterFunction($matches[1], $matches[2]);
				
			//stop condition
			if(strpos($line, "RegisterOLDScript") !== false)
			{
				$this->last_line = $line_number;
				break;
			}

			$line_number++;
		}
		
		if($this->last_line === false)
			throw new Exception("OldScriptRegistering: Could not find last line in registering for script $script_name");
	}
	
	private function RegisterFunction($interface_function, $function_name) : void
	{
		echo "Registering function: $function_name ($interface_function)".PHP_EOL;
		switch($interface_function) {
			case "GetAI":
				$this->getAIFunction = $function_name;
				break;
			default:
				$this->otherFunctions[$interface_function] = $function_name;
				break;
		}
	}
	
	//return name of GetAI function, or false
	public function GetAIFunction() : ?string
	{
		return $this->getAIFunction;
	}
	
	//return array of names of all functions other than GetAI
	public function GetOtherFunctions() : array
	{
		return $this->otherFunctions;
	}

	//return first and last line. False if not found
	public function GetBoundaries() : array
	{
		return array($this->first_line, $this->last_line);
	}
}

class FunctionHelper
{
	static public function RemoveBlock(array &$lines, $block_name) : void
	{
		//echo "Removing block $block_name".PHP_EOL;
		$found_block_start = false;
		$indent_level = 0;
		$first_loop = true;
		$delete_count = 0;
		foreach($lines as $key => &$line) {
			//loop until we find block start
			if($found_block_start === false) {
				if(strpos($line, $block_name.'(') !== false || strpos($line, $block_name.' :') !== false)
					$found_block_start = true;
				else 
					continue;
			}
			
			$indent_level += substr_count($line, '{');
			$indent_level -= substr_count($line, '}');
			
			//delete current line
			$delete_count++;
			//echo "($indent_level) $line";
			unset($lines[$key]);
			
			//check function end
			if($first_loop !== true && $indent_level == 0)
				break;
			
			$first_loop = false;
		}
		//echo "Removed $delete_count lines".PHP_EOL;
	}
	
	static public function GetBlockLine(array &$lines, $block_name) : ?int
	{
		$line_count = 0;
		foreach($lines as $line)
		{
			if(strpos($line, $block_name.'(') === false && strpos($line, $block_name.' :') === false) {
				$line_count++;
				continue;
			}
			
			return $line_count;
		}
		return null;
	}
	
	//return array containing function arguments names
	static public function GetFunctionArguments(array &$lines, $function_name) : ?array
	{
		$results = array();
		foreach($lines as $line)
		{
			if(strpos($line, $function_name.'(') === false)
				continue;
			
			//this suppose the definition is done in one line
			$matches = array();
			preg_match('/'."$function_name\((.+)\)".'/', $line, $matches);
			$full_arg_definitions = explode(',', $matches[1]);
			foreach($full_arg_definitions as $full_arg) {
				$full_arg = trim($full_arg);
				//remove stars
				$full_arg = str_replace('*', ' ', $full_arg);
				$matches2 = array();
				preg_match('/ ([^ ]+)$/', $full_arg, $matches2);
				if($matches2[1] === $full_arg)
					throw new Exception("Could not extract argument name from string $full_arg");
				array_push($results, $matches2[1]);
				//echo "Converted arg '$full_arg' to '".$matches2[1]."'".PHP_EOL;
			}
			return $results;
		}
		//did not find any function
		return null;
	}
	
	static public function GetFunctionContent(array &$lines, $function_name) : array
	{
		$results = array();
		
		//we'll first contact the whole function in one string to be able to regex on it
		$function_concat = false;
		$found_function_decl = false; //start looking for {
		$started_recording = false;
		$indent_level = 0;
		foreach($lines as $line) {
			//loop until we find func decl line
			if($found_function_decl === false) {
				if(strpos($line, $function_name.'(') !== false)
					$found_function_decl = true;
				else 
					continue;
			}
			
			$indent_level += substr_count($line, '{');
			if($indent_level > 0)
				$started_recording = true;
			$indent_level -= substr_count($line, '}');
			
			$function_concat .= $line;
			
			//check function end
			if($started_recording && $indent_level == 0)
				break;
		}
		
		//echo "Found function $function_name with def: ".PHP_EOL;
		//print_r($function_concat);
		//use regex magic to get function here
		$matches = array();
		preg_match('/^[^{]*{(.*)}[^}]*$/s', $function_concat, $matches);
		
		//aaand lets put it back in lines format
		$results = preg_split('/\r\n|\r|\n/', $matches[1]);
		//print_r($results);
		//remove empty lines at begin and end
		if($results[0] == "")
			unset($results[0]);
				
		if($results[count($results)-1] == "")
			unset($results[count($results)]);
		
		//aaand re add line returns
		foreach($results as &$line) {
			$line .= PHP_EOL;
		}
		
		return $results;
	}
	
	static public function GetLineIndent(&$line) : ?int
	{
		//...not very performant
		$arr = str_split($line);
		$pos = 0;
		foreach($arr as $char) {
			if($char !== ' ')
				return $pos;
			$pos++;
		}
		return null; //not found
	}
	
	//set indent to given indent. First line of content is used as reference for current indent
	static public function ResetIndent(array &$content, $indent) : void
	{
		$indent_str = str_repeat(" ", $indent);
		$starting_indent = self::GetLineIndent($content[1]);
		//echo "Starting line indent: $starting_indent" .PHP_EOL;
		
		//for each line, remove all indent and replace with given indent
		foreach($content as &$line) {
			$line = preg_replace('/^[ ]{'.$starting_indent.'}(.*)$/', $indent_str.'$1', $line);
		}
		
		//print_r($content);
	}

	//replace pointer variable with a new name... this is very approximative and won't cover all cases, but i'm not writing a full c++ parser
	static public function ReplacePointer(array &$content, $replace_me, $replace_with) : void
	{
		foreach($content as &$line) {
			//replace X-> with Y-> in code
			$line = str_replace($replace_me.'->', $replace_with.'->', $line);
			//replace function argument
			$line = str_replace(" $replace_me,", " $replace_with,", $line);
			$line = str_replace(" $replace_me)", " $replace_with)", $line);
			$line = str_replace(" $replace_me )", " $replace_with )", $line);
		}
	}
}

class OldScriptAIStruct
{
	private $lines;
	private $ai_struct_name;
	private $found_struct;
	
	function __construct($ai_function_name, array &$lines)
	{
		$this->lines = $lines;
		$this->ai_struct_name = $this->_GetAIStructName($ai_function_name);
	}
	
	public function GetAIStructName() : string 
	{
		return $this->ai_struct_name;
	}
	
	private function _GetAIStructName($ai_function_name) : string
	{
		$matches = array();
		$start_searching = false;
		$search_count = 0;
		//find begin line
		foreach($this->lines as $line) {
			//go to starting line
			if($start_searching === false && strpos($line, $ai_function_name . '(Creature') !== false) {
				$start_searching = true;
				continue;
			}
				
			if($start_searching === false)
				continue;
						
			//try getting the struct name
			$match = preg_match('/new (\w+)[ ]*\(/', $line, $matches);
			if($match) {
				if($matches[1] == "SimpleAI" || $matches[1] == "guardAI")
					throw new Exception("OldScriptAIStruct found a ".$matches[1]." AI, I don't know how to handle this one");
				
				return $matches[1];
			}
			
			//else we continue for 5 times
			$search_count++;
			if($search_count > 5)
				throw new Exception("OldScriptAIStruct could not find AI structure name with AI function name $ai_function_name");
		}
		
		throw new Exception("OldScriptAIStruct could not find the function with name $ai_function_name");
	}
	
	public function GetContent() : array
	{
		$results = array();
		$indent_level = 0;
		$found_struct = false;
		$started_recording = false;
		$skip = false;
		$insert_public = false; //insert public keyword after loop
		foreach($this->lines as &$line) {
			
			//wait until we find struct as well
			if($found_struct === false) {
				if(preg_match('/struct[ ]*'.$this->ai_struct_name.'[ ]*:/', $line)) {
					$found_struct = true;
					//replace struct by class
					$line = str_replace('struct ', 'class ', $line);
					$insert_public = true;
				} else {
					continue;
				}
			}
			
			//echo "($indent_level) processing line $line" . PHP_EOL;
			
			//handle indent levels
			$indent_level += substr_count($line, '{');
			if($indent_level > 0)
				$started_recording = true;
			$indent_level -= substr_count($line, '}');
			
			//check struct end
			if($started_recording && $indent_level == 0) {
				/*echo "Finished at line: " . PHP_EOL;
				echo $line . PHP_EOL;*/
				break;
			}
			
			array_push($results, $line);
			if($insert_public && $indent_level == 1) {
				array_push($results, '    public:'.PHP_EOL); //will probably be wrongly indented
				$insert_public = false;
			}
		}
		
		//remove empty lines at beginning
		foreach($results as $key => $result) {
			if(trim($result) == "")
				unset($results[$key]);
			else 
				break;
		}
		
		//replace some functions
		foreach($results as $key => &$line) {
			if(strpos($line, "OnGossipHello") !== false) {
				echo "TRYING TO REPLACE ON GOSSIP HELLO" . PHP_EOL;
				$line = preg_replace('/OnGossipHello\((.+),(.+)\)/', 'GossipHello($1)', $line);
			}
		}
		
		return $results;
	}
}

class OldScript
{
	private $script_name;
	private $ai_name;
	private $content;
	private $new_script_insert_position = null;
	
	//lines from ScriptedAI with corrected indentation
	private $ai_functions;
	
	private $registering;
	private $ai_struct = null;
	private $ai_struct_errored = false;
	
	private $lines;
	
	//can throw on error
	function __construct($name, array &$lines)
	{
		echo "Constructing model for script $name" . PHP_EOL;
		$this->script_name = $name;
		
		$this->registering = new OldScriptRegistering($name, $lines);
		$ai_function_name = $this->registering->GetAIFunction();
		if($ai_function_name !== null) {
			try {
				$this->ai_struct = new OldScriptAIStruct($ai_function_name, $lines);
			} catch (Exception $e) {
				echo "Couldn't find struct for ai name $ai_function_name. Error was: ". $e->getMessage() . PHP_EOL;
				$this->ai_struct_errored = true;
			}
		}
		
		$this->lines = $lines;
	}
	
	public function GetRegistering() : OldScriptRegistering
	{
		return $this->registering;
	}
	
	private function GetNewScriptInsertPosition() : int
	{
		if($new_script_insert_position === null)
			throw new Exception("GetNewScriptInsertPosition has no position to give");
		
		return $new_script_insert_position;
	}
	
	//insert position = the lowest line where we removed something
	//update position if given line is lower than currently existing
	private function UpdateScriptInsertPosition($line) : void
	{
		if($this->new_script_insert_position === null || $line < $this->new_script_insert_position)
			$this->new_script_insert_position = $line;
	}
	
	public function ReplaceOldScript(array &$content) : void
	{
		//generate new script
		$new_script = $this->Convert();
		
		//update insert position if needed
		$ai_function_name = $this->registering->GetAIFunction();
		if($ai_function_name !== null) {
			$function_line = FunctionHelper::GetBlockLine($this->lines, $ai_function_name);
			$this->UpdateScriptInsertPosition($function_line);
		}

		if($this->new_script_insert_position == 0)
			throw new Exception("ReplaceOldScript: Did not find any replace position");
		
		//insert new script
		echo "Inserting new script at line: " . $this->new_script_insert_position . PHP_EOL;
		ScriptConverter::InsertFromLine($content, $this->new_script_insert_position, $new_script);
		
		//remove old script (must be after inserting the new since to not alter insert position
		echo "Cleaning old script function..." . PHP_EOL;
		$this->RemoveOldScript($content);
	}
	
	private function RemoveOldScript(array &$content) : void
	{
		$remove_me = array(); //function or struct names to remove
		
		$ai_function_name = $this->registering->GetAIFunction();
		if($ai_function_name !== null && $this->ai_struct_errored === false)
			array_push($remove_me, $ai_function_name);
		
		$other_funcs = $this->registering->GetOtherFunctions();
		$remove_me = array_merge($remove_me, $other_funcs); 
		
		if($this->ai_struct !== null)
			array_push($remove_me, $this->ai_struct->GetAIStructName());
		
		foreach($remove_me as $remove) {
			FunctionHelper::RemoveBlock($content, $remove);
		}
	}
	
	//return new script, in the form of an array of lines
	private function Convert() : array
	{
		//just for convenience
		$script_name = $this->script_name;
		
		$new_script = [
			"class $script_name : public CreatureScript"                      .PHP_EOL,
			"{"                                                               .PHP_EOL,
			"public:"                                                         .PHP_EOL,
			"    $script_name() : CreatureScript(\"$script_name\")"           .PHP_EOL,
			"    { }"                                                         .PHP_EOL,
			""                                                                .PHP_EOL,
		];
		
		$new_script = array_merge($new_script, $this->GetAIFunctionHelper());
		$ai_name = $this->ai_name; //may have been altered in GetAIFunctionHelper
		$new_script = array_merge($new_script, [
		"    CreatureAI* GetAI(Creature* creature) const override"    .PHP_EOL,    
		"    {"                                                       .PHP_EOL,
		"        return new $ai_name(creature);"                      .PHP_EOL,
		"    }"                                                       .PHP_EOL,
		]);
	
		//closing brace
		$new_script = array_merge($new_script, array("};".PHP_EOL.PHP_EOL));
		
		$this->content = $new_script;
		return $this->content;
	}
	
	private function GetAIFunctionHelper() : array
	{
		//just for convenience
		
		$ai_part = array();
		
		if($this->ai_struct === null) {
			$this->ai_name = $this->script_name . 'AI';
			$ai_name = $this->ai_name; //just to simplify next array definition
			$ai_part = [
				"    class $ai_name : public ScriptedAI"                      .PHP_EOL,
				"    {"                                                       .PHP_EOL,
				"    public:"                                                 .PHP_EOL,
				"        $ai_name(Creature* creature) : ScriptedAI(creature)" .PHP_EOL,
				"        {}"                                                  .PHP_EOL,
				""                                                                .PHP_EOL,
			];
			if($this->ai_struct_errored) {
				//insert a voluntary C++ error here to avoid compilation passing without noticing
				$ai_part = array_merge($ai_part, array("ERRORME"));
			}
		}
		
		if($this->ai_struct !== null) {
			$this->ai_name = $this->ai_struct->GetAIStructName();
			$content = $this->ai_struct->GetContent();
			FunctionHelper::ResetIndent($content, 4);
			$ai_part = array_merge($ai_part, $content);
		}
		
		foreach($this->registering->GetOtherFunctions() as $interface_name => $function_name) {
			$content = $this->GetFunctionHelper($interface_name, $function_name);
			FunctionHelper::ResetIndent($content, 8);
			$ai_part = array_merge($ai_part, array(PHP_EOL), $content);
		}
		
		$ai_part = array_merge($ai_part, [
			"    };"                                                      .PHP_EOL,
			""                                                                .PHP_EOL,
		]);
		
		return $ai_part;
	}
	
	//convert old function to new format and give it proper indentation
	//note that some variable names wont be converted, it's really complicated to handle. Those will create compiler error to fix by hand.
	private function GetFunctionHelper($interface_name, $function_name) : array
	{
		echo "Converting $function_name ..." . PHP_EOL;
		//update insert position if needed
		$function_line = FunctionHelper::GetBlockLine($this->lines, $function_name);
		$this->UpdateScriptInsertPosition($function_line);
		
		//get function arguments names to place them in new function definition
		$function_arguments = FunctionHelper::GetFunctionArguments($this->lines, $function_name);
		$function_content = FunctionHelper::GetFunctionContent($this->lines, $function_name);
		$function_def = "";
		$creature_pointer_to_replace = null; //replace this variable name by "me" if any
		switch($interface_name)
		{
		case "OnGossipHello":
			$function_def = [
				"                virtual bool GossipHello(Player* ".$function_arguments[0].") override"                                                .PHP_EOL,
			];
			$creature_pointer_to_replace = $function_arguments[1];
			break;
		case "OnGossipSelect":
			$function_def = [
				"                virtual bool GossipSelect(Player* ".$function_arguments[0].", uint32 ".$function_arguments[2].", uint32 ".$function_arguments[3].") override"     .PHP_EOL,
			];
			$creature_pointer_to_replace = $function_arguments[1];
			break;
		case "OnQuestAccept":
			$function_def = [
				"                virtual void QuestAccept(Player* ".$function_arguments[0].", Quest const* ".$function_arguments[2].") override"                                                 .PHP_EOL,
			];
			$creature_pointer_to_replace = $function_arguments[1];
			break;
		case "OnQuestReward":
			$function_def = [
				"                virtual void QuestReward(Player* ".$function_arguments[0].", Quest const* ".$function_arguments[2].", uint32 ".$function_arguments[3].") override"                                 .PHP_EOL,
			];
			$creature_pointer_to_replace = $function_arguments[1];
			break;
		case "OnQuestComplete": //replace with reward
			$function_def = [
				"                virtual void QuestReward(Player* ".$function_arguments[0].", Quest const* ".$function_arguments[2].", uint32 /*opt*/) override"                                 .PHP_EOL,
			];
			$creature_pointer_to_replace = $function_arguments[1];
			break;
		case "OnReceiveEmote":
			$function_def = [
				"                virtual void ReceiveEmote(Player* ".$function_arguments[0].", uint32 ".$function_arguments[2].") override"                                                 .PHP_EOL,
			];
			$creature_pointer_to_replace = $function_arguments[1];
			break;
		case "OnGossipSelectCode":
			$function_def = [
				"                virtual bool GossipSelectCode(Player* ".$function_arguments[0].", uint32 ".$function_arguments[2].", uint32 ".$function_arguments[3].", const char* ".$function_arguments[4].") override"                                                 .PHP_EOL,
			];
			break;
			break;
		case "OnEffectDummyCreature":
			$function_def = [
				"                virtual bool sOnDummyEffect(Player* ".$function_arguments[0].", uint32 ".$function_arguments[1].", uint32 ".$function_arguments[2].") override"                                                 .PHP_EOL,
			];		
			break;
		
		default:
			throw new Exception( "Unknown interface function name $interface_name, I don't know how to handle this one, cannot continue");
		};
		$function_def = array_merge($function_def, [
			"                {" .PHP_EOL,
		]);
		FunctionHelper::ResetIndent($function_content, 20);
		if($creature_pointer_to_replace !== null)
			FunctionHelper::ReplacePointer($function_content, $creature_pointer_to_replace, 'me');
		
		$function_def = array_merge($function_def, $function_content);
		
		$function_def = array_merge($function_def, [
			"                }" .PHP_EOL,
			                     PHP_EOL,
		]);
		return $function_def;
	}
}