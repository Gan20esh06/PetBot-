# Wake Word Troubleshooting & Expression Guide

## 🎤 Wake Word Detection - FIXED!

### What Changed:

1. **Better Detection Algorithm**

   - Now detects partial wake word matches
   - More flexible recognition (e.g., "hey" or "robot" triggers it)
   - Better error handling
   - Auto-restart when recognition ends

2. **Visual Feedback**

   - Wake word mode: Subtle blue pulse on avatar
   - Listening mode: Bright blue glowing eyes (70px size)
   - Processing mode: Purple thinking eyes
   - Speaking mode: Green eyes with moving pupils

3. **Improved Logging**
   - Shows everything it hears: `👂 Heard: "..."`
   - Shows when wake word detected
   - Shows when restarting detection
   - Click "Show Log" to see all activity

## 🎨 New Facial Expressions

### 1. Wake Word Listening (Default State)

- **Eyes:** Normal dark eyes with natural blinking
- **Face:** Subtle blue pulse around edges
- **Mouth:** Normal smile
- **Status:** "🎤 Say 'hey robot' to wake me up"

### 2. Listening (After Wake Word)

- **Eyes:** BRIGHT BLUE glowing with pulse animation (70px)
- **Pupils:** Enlarged and glowing
- **Mouth:** Open wider (140px), pulsing up and down
- **Face:** Slightly larger (scale 1.05)
- **Status:** "👂 Listening..."
- **Duration:** Until you finish speaking

### 3. Processing (Thinking)

- **Eyes:** PURPLE with gentle pulse
- **Pupils:** Moving slightly
- **Mouth:** Smaller (100px), wiggling side to side
- **Status:** "🧠 Thinking..."
- **Duration:** While calling OpenAI API

### 4. Speaking (Answering)

- **Eyes:** GREEN
- **Pupils:** Moving left and right with speech
- **Mouth:** Animated speech movements (varying heights)
- **Status:** "🗣️ Speaking..."
- **Duration:** While avatar is talking

## 🔧 How to Test Wake Word

### Method 1: Quick Test

1. Open avatar in browser
2. Look for status: "🎤 Say 'hey robot' to wake me up"
3. **Say clearly:** "Hey robot"
4. Watch for:
   - Green flash at top: "🎤 Wake word detected!"
   - Eyes instantly turn BRIGHT BLUE
   - Status changes to "👂 Listening..."

### Method 2: Check Logs

1. Click "Show Log" button (bottom right)
2. Look for:
   ```
   👂 Wake word listening started - say: hey robot
   👂 Wake word detection active
   👂 Heard: "hey robot"
   🎤 Wake word detected: hey robot
   ```

### Method 3: Alternative Wake Words

If "hey robot" isn't working, try:

1. Change wake word in settings to:
   - "hello robot"
   - "hey buddy"
   - "wake up"
   - Any 1-3 word phrase
2. Save settings
3. Try the new phrase

## 🐛 Common Issues & Fixes

### Issue 1: "Wake word not detecting at all"

**Check:**

- ✅ Microphone permission granted (look for 🎤 in browser address bar)
- ✅ Not muted or too quiet
- ✅ Using Chrome or Edge browser
- ✅ Check log shows "👂 Wake word listening started"

**Fix:**

1. Reload page (Ctrl+R or F5)
2. Click "Allow" when browser asks for microphone
3. Speak at normal volume, clearly
4. Check log console for errors

### Issue 2: "Wake word detected but doesn't listen"

**Check:**

- Log shows "🎤 Wake word detected!"
- Eyes turn blue briefly but stop

**Fix:**

- This is normal! Now say your command immediately
- Eyes turn blue = it's listening for 5-10 seconds
- Don't wait too long after saying wake word

### Issue 3: "Eyes don't change color"

**Check:**

- Look at status text - does it change?
- Browser console (F12) for CSS errors

**Fix:**

1. Hard reload: Ctrl+Shift+R
2. Clear browser cache
3. Check CSS loaded properly

### Issue 4: "Detection stops after a while"

**Check:**

- Log shows "🔄 Wake word recognition ended"
- Should auto-restart after 300ms

**Fix:**

- Normal behavior - it restarts automatically
- If it doesn't restart, check logs for errors
- May need microphone permission again

### Issue 5: "Detects wrong words"

**Example:** Triggers on "a robot" or "may prop it"

**Fix:**

1. Change wake word to something more unique:
   - "activate assistant"
   - "hello jarvis"
   - "computer wake up"
2. Use 3+ word phrases for better accuracy
3. Speak slowly and clearly

## 💡 Pro Tips for Best Detection

### Speaking Technique:

1. **Pause before wake word** - Give it 1 second of silence
2. **Speak clearly** - Normal conversational pace
3. **Normal volume** - Don't whisper or shout
4. **Distinct pronunciation** - "HEY... RO-BOT"
5. **Consistent** - Say it the same way each time

### Environment:

1. **Quiet room** - Background noise reduces accuracy
2. **Close to mic** - Within 1-2 meters
3. **No echo** - Soft furnishings help
4. **Good connection** - Speech API needs internet

### Settings:

1. **Short wake words work better** - 2 words ideal
2. **Common words** - "hey", "hello", "computer"
3. **Avoid similar phrases** - Don't use words you say often
4. **Test variations** - Try different phrasing

## 🎯 Expected Behavior

### Normal Flow:

```
1. Avatar loads
   Status: "🎤 Say 'hey robot' to wake me up"
   Eyes: Normal, blinking
   Log: "👂 Wake word listening started"

2. You say: "Hey robot"
   Log: "👂 Heard: 'hey robot'"
   Log: "🎤 Wake word detected: hey robot"
   Status: "👂 Listening..."
   Eyes: BRIGHT BLUE, glowing
   Indicator: Green flash

3. You say: "What is photosynthesis?"
   Log: "📝 Final transcript: What is photosynthesis?"
   Status: "🧠 Thinking..."
   Eyes: PURPLE, pulsing
   API: Calling OpenAI...

4. Avatar responds
   Status: "🗣️ Speaking..."
   Eyes: GREEN
   Mouth: Animated speech
   Voice: Speaking answer

5. Returns to wake word mode
   Status: "🎤 Say 'hey robot' to wake me up"
   Eyes: Normal
   Log: "🔄 Restarting wake word detection..."
```

## 📊 Expression Timeline

| Time | State               | Eyes      | Mouth       | Face         |
| ---- | ------------------- | --------- | ----------- | ------------ |
| 0:00 | Wake listening      | Dark      | Smile       | Subtle pulse |
| 0:05 | (You say wake word) | -         | -           | -            |
| 0:06 | Listening           | BLUE 70px | Open 140px  | Scale 1.05   |
| 0:10 | (You speak command) | BLUE      | Pulsing     | Glowing      |
| 0:15 | Processing          | PURPLE    | Small 100px | Normal       |
| 0:18 | Speaking            | GREEN     | Animated    | Normal       |
| 0:25 | Wake listening      | Dark      | Smile       | Subtle pulse |

## 🧪 Testing Checklist

- [ ] Page loads without errors
- [ ] Status shows wake word prompt
- [ ] Log shows "Wake word listening started"
- [ ] Microphone permission granted
- [ ] Say wake word clearly
- [ ] Green flash appears
- [ ] Eyes turn BRIGHT BLUE
- [ ] Status shows "Listening..."
- [ ] Say a command
- [ ] Eyes turn PURPLE (thinking)
- [ ] Eyes turn GREEN (speaking)
- [ ] Avatar speaks response
- [ ] Returns to wake word mode
- [ ] Can repeat the process

## 🔍 Debug Commands

Open browser console (F12) and type:

```javascript
// Check current state
console.log("Current state:", currentState);

// Check wake word
console.log("Wake word:", CONFIG.wakeWord);

// Manual trigger
onWakeWordDetected();

// Check recognition
console.log("Wake word recognition:", wakeWordRecognition);
console.log("Main recognition:", recognition);

// Test state change
setState(State.LISTENING);
setState(State.PROCESSING);
setState(State.SPEAKING);
```

## 🎉 Success Indicators

You know it's working when:

1. ✅ Log shows everything it hears
2. ✅ Green flash when wake word detected
3. ✅ Eyes change color at each state
4. ✅ Status updates correctly
5. ✅ Avatar responds with voice
6. ✅ Auto-returns to wake word mode
7. ✅ Can repeat multiple times

## 📱 Mobile Considerations

On Android Chrome:

- Wake word works great
- Better microphone
- Lower latency
- Touch-friendly settings

On iOS:

- ❌ Wake word NOT supported (Safari limitation)
- ❌ Web Bluetooth NOT supported
- Use desktop or Android instead

## 🚀 Advanced Tips

### Custom Wake Word Examples:

- "Hey buddy" - Friendly
- "Computer" - Like Star Trek
- "Jarvis" - Like Iron Man
- "Robot activate" - Clear command
- "Hello assistant" - Professional

### Multiple Wake Words:

Change code to accept variations:

```javascript
const wakeWords = ["hey robot", "hello robot", "hi robot"];
if (wakeWords.some((word) => transcript.includes(word))) {
  onWakeWordDetected();
}
```

### Sensitivity Tuning:

Adjust matching threshold:

```javascript
// Current: 50% of words must match
if (matchCount >= Math.ceil(wakeWords.length / 2))

// Stricter: All words must match
if (matchCount === wakeWords.length)

// Looser: Any word matches
if (matchCount >= 1)
```

---

**Your wake word detection is now fixed and working!**  
**Expressions are dynamic and show the avatar's state!**  
**Just say "Hey Robot" and watch the magic! 🤖✨**
